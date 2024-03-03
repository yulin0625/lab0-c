#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head) {
        return NULL;
    }

    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *safe, *it;
    list_for_each_entry_safe (it, safe, head, list)
        q_release_element(it);

    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }

    element_t *new_element = malloc(sizeof(element_t));
    if (!new_element) {
        return false;
    }

    new_element->value = strdup(s);
    if (!new_element->value) {
        free(new_element);
        return false;
    }

    list_add(&new_element->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }

    return q_insert_head(head->prev, s);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }

    element_t *element = list_first_entry(head, element_t, list);
    list_del(&element->list);

    if (sp && bufsize) {
        strncpy(sp, element->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return element;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }

    element_t *element = list_last_entry(head, element_t, list);
    list_del(&element->list);

    if (sp && bufsize) {
        strncpy(sp, element->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return element;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head *slow = head->next;
    for (struct list_head *fast = head->next;
         (fast != head) && (fast->next != head); fast = fast->next->next) {
        slow = slow->next;
    }
    element_t *mid_element = list_entry(slow, element_t, list);
    list_del(&mid_element->list);
    q_release_element(mid_element);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;

    element_t *it, *safe;
    bool equal = false;
    list_for_each_entry_safe (it, safe, head, list) {
        if ((&safe->list != head) &&
            !strcmp(
                it->value,
                safe->value)) {  // If the s1 and s2 are equal, strcmp return 0.
            list_del(&it->list);
            q_release_element(it);
            equal = true;
        } else if (equal) {
            list_del(&it->list);
            q_release_element(it);
            equal = false;
        }
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    q_reverseK(head, 2);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head) {
        return;
    }

    element_t *it, *safe;
    list_for_each_entry_safe (it, safe, head, list) {
        list_move(&it->list, head);
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head)) {
        return;
    }

    struct list_head *it, *safe, *cut = head;

    int cnt = 0;

    list_for_each_safe (it, safe, head) {
        cnt++;
        if (cnt != k) {
            continue;
        }

        LIST_HEAD(tmp);
        list_cut_position(&tmp, cut, it);
        q_reverse(&tmp);
        list_splice(&tmp, cut);
        cnt = 0;
        cut = safe->prev;
    }
}

int mergeTwoList(struct list_head *L1, struct list_head *L2, bool descend)
{
    if (!L1 || !L2)
        return 0;

    LIST_HEAD(tmp);
    int cnt = 0;

    while (!list_empty(L1) && !list_empty(L2)) {
        element_t *e1 = list_first_entry(L1, element_t, list);
        element_t *e2 = list_first_entry(L2, element_t, list);
        int cmp = strcmp(e1->value, e2->value);
        if (descend) {
            cmp = -cmp;
        }

        if (cmp > 0) {
            list_move_tail(L2->next, &tmp);  // move L2 first node to tmp
        } else {
            list_move_tail(L1->next, &tmp);  // move L1 first node to tmp
        }
        cnt++;
    }
    cnt += q_size(L1) + q_size(L2);
    list_splice(&tmp, L1);
    list_splice_tail(L2, L1);
    return cnt;
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head)) {
        return;
    }

    struct list_head *mid, *left, *right;
    left = right = head;
    do {
        left = left->next;
        right = right->prev;
    } while (left != right && left->next != right);
    mid = left;

    LIST_HEAD(second);
    list_cut_position(&second, mid, head->prev);

    q_sort(head, descend);
    q_sort(&second, descend);

    mergeTwoList(head, &second, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head)) {
        return 0;
    }

    element_t *curr = list_last_entry(head, element_t, list);
    int cnt = 1;

    while (curr->list.prev != head) {
        element_t *prev = list_entry(curr->list.prev, element_t, list);
        if (strcmp(curr->value, prev->value) < 0) {
            list_del(&prev->list);
            q_release_element(prev);
        } else {
            cnt++;
            curr = prev;
        }
    }
    return cnt;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head)) {
        return 0;
    }

    element_t *curr = list_last_entry(head, element_t, list);
    int cnt = 1;

    while (curr->list.prev != head) {
        element_t *prev = list_entry(curr->list.prev, element_t, list);
        if (strcmp(curr->value, prev->value) > 0) {
            list_del(&prev->list);
            q_release_element(prev);
        } else {
            cnt++;
            curr = prev;
        }
    }
    return cnt;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
