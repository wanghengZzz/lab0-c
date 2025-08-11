#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new_q =
        (struct list_head *) malloc(sizeof(struct list_head));
    if (!new_q)
        return NULL;
    INIT_LIST_HEAD(new_q);
    return new_q;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    struct list_head *node, *next_node;
    if (!head)
        return;
    list_for_each_safe(node, next_node, head)
        q_release_element(list_entry(node, element_t, list));
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_node = (element_t *) malloc(sizeof(element_t));
    if (!new_node)
        return false;
    new_node->value = strdup(s);
    if (!new_node->value) {
        free(new_node);
        return false;
    }
    list_add(&new_node->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_node = (element_t *) malloc(sizeof(element_t));
    if (!new_node)
        return false;
    new_node->value = strdup(s);
    if (!new_node->value) {
        free(new_node);
        return false;
    }
    list_add_tail(&new_node->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *remove_node = list_first_entry(head, element_t, list);
    list_del(head->next);
    if (sp && bufsize) {
        strncpy(sp, remove_node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }


    return remove_node;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *remove_node = list_last_entry(head, element_t, list);
    list_del(head->prev);
    if (sp && bufsize) {
        strncpy(sp, remove_node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return remove_node;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;
    list_for_each(li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head *slow = head->next, *fast = slow;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    list_del(slow);
    element_t *del_node = container_of(slow, element_t, list);
    q_release_element(del_node);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;
    if (list_is_singular(head))
        return true;

    struct list_head *curr = head->next, *tail_node = head;
    element_t *last_node = list_entry(curr, element_t, list);
    bool dup = false;
    while (curr->next != head) {
        curr = curr->next;
        element_t *curr_entry = list_entry(curr, element_t, list);
        if (!strcmp(curr_entry->value, last_node->value)) {
            dup = true;
            q_release_element(last_node);
        } else {
            if (!dup) {
                tail_node->next = &last_node->list;
                last_node->list.prev = tail_node;
                tail_node = &last_node->list;
            } else {
                q_release_element(last_node);
            }
            dup = false;
        }
        last_node = curr_entry;
    }
    if (!dup) {
        tail_node->next = &last_node->list;
        last_node->list.prev = tail_node;
    } else {
        head->prev = tail_node;
        tail_node->next = head;
        q_release_element(last_node);
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
    if (!head || list_empty(head))
        return;

    struct list_head *curr, *safe;
    list_for_each_safe(curr, safe, head)
        list_move(curr, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    struct list_head *left, *right, *prev, *middle, *next, *last_group_head;
    left = right = head->next;
    last_group_head = NULL;
    bool first_reverse = true;

    while (right != head) {
        int curr_k = k;
        while (curr_k > 1) {
            if (right) {
                right = right->next;
                curr_k--;
            }
            if (right == head)
                return;
        }
        prev = right->next;
        middle = next = left;
        right = right->next;
        do {
            next = next->next;
            middle->next = prev;
            prev->prev = middle;
            prev = middle;
            middle = next;
        } while (next != right);
        if (last_group_head) {
            last_group_head->next = prev;
            prev->prev = last_group_head;
        }
        last_group_head = left, left = right;
        if (first_reverse) {
            first_reverse = false;
            head->next = prev;
            prev->prev = head;
        }
    }
}

/* Sort elements of queue in ascending/descending order */

void two_way_merge(struct list_head *run1, struct list_head *run2, bool descend)
{
    if (!run1 || !run2)
        return;
    LIST_HEAD(tmp_head);

    while (!list_empty(run1) && !list_empty(run2)) {
        const element_t *r1_entry = list_first_entry(run1, element_t, list);
        const element_t *r2_entry = list_first_entry(run2, element_t, list);
        if ((!descend && strcmp(r1_entry->value, r2_entry->value) <= 0) ||
            (descend && strcmp(r2_entry->value, r2_entry->value) >= 0)) {
            list_move_tail(run1->next, &tmp_head);
        } else
            list_move_tail(run2->next, &tmp_head);
    }

    list_splice(&tmp_head, run1);
    list_splice_tail_init(run2, run1);
}

void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *slow = head->next, *fast = slow;
    for (; fast != head && fast->next != head;
         slow = slow->next, fast = fast->next->next)
        ;

    LIST_HEAD(rhs_head);
    struct list_head *lhs_last = slow->prev;
    rhs_head.next = slow;
    head->prev->next = &rhs_head;
    rhs_head.prev = head->prev;
    slow->prev = &rhs_head;

    lhs_last->next = head;
    head->prev = lhs_last;
    q_sort(head, descend);
    q_sort(&rhs_head, descend);
    two_way_merge(head, &rhs_head, descend);
}


/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    q_reverse(head);
    const char *less_str = list_entry(head->next, element_t, list)->value;

    struct list_head *trace = head->next->next;
    struct list_head *delete_node = NULL;
    while (trace != head) {
        char *trace_val = list_entry(trace, element_t, list)->value;
        if (strcmp(trace_val, less_str) > 0) {
            delete_node = trace;
            trace = trace->next;
            list_del(delete_node);
            q_release_element(list_entry(delete_node, element_t, list));
        } else {
            less_str = trace_val;
            trace = trace->next;
        }
    }
    q_reverse(head);
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    q_reverse(head);
    const char *greatest_str = list_entry(head->next, element_t, list)->value;

    struct list_head *trace = head->next->next;
    struct list_head *delete_node = NULL;
    while (trace != head) {
        char *trace_val = list_entry(trace, element_t, list)->value;
        if (strcmp(trace_val, greatest_str) < 0) {
            delete_node = trace;
            trace = trace->next;
            list_del(delete_node);
            q_release_element(list_entry(delete_node, element_t, list));
        } else {
            greatest_str = trace_val;
            trace = trace->next;
        }
    }
    q_reverse(head);
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */

void q_partition(struct list_head *head, int left, int right, bool descend)
{
    if (right > left) {
        int cut = (right - left) / 2;
        int m = left + cut;
        struct list_head *cur = head;
        while (cut-- > 0)
            cur = cur->next;
        q_partition(head, left, m, descend);
        q_partition(cur->next, m + 1, right, descend);
        queue_contex_t *ql = container_of(head, queue_contex_t, chain);
        queue_contex_t *qr = container_of(cur->next, queue_contex_t, chain);
        qr->size = 0;
        two_way_merge(ql->q, qr->q, descend);
    }
}

int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;
    // https://leetcode.com/problems/merge-k-sorted-lists/
    int size_of_q = 0;
    struct list_head *cur;
    list_for_each(cur, head)
        size_of_q += 1;
    q_partition(head->next, 0, size_of_q - 1, descend);
    queue_contex_t *first_queue =
        container_of(head->next, queue_contex_t, chain);
    first_queue->size = q_size(first_queue->q);
    return first_queue->size;
}
