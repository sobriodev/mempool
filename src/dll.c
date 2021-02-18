#include "dll.h"

/* ------------------------------------------------------------ */
/* ---------------------- Private data types ------------------ */
/* ------------------------------------------------------------ */

typedef enum boundary_type_
{
    boundary_head = 0,
    boundary_tail = 1,
} boundary_type;

/* ------------------------------------------------------------ */
/* ---------------------- Private functions ------------------- */
/* ------------------------------------------------------------ */

static void set_status(dll_status status, dll_status* out)
{
    if (LIKELY(NULL != out)) {
        *out = status;
    }
}

#if DLL_NEW_NODE_SANITY_CHECK
/* The function expects a valid pointer */
static inline bool new_node_sanity_check(dll_node* node)
{
    return (NULL == node->prev) && (NULL == node->next);
}
#endif

#if DLL_HEAD_SANITY_CHECK
/* The function expects a valid pointer */
static inline bool head_node_sanity_check(dll_node* head)
{
    return (NULL == head->prev);
}
#endif

static dll_node* find_boundary(dll_node* node, boundary_type boundary)
{
    if (UNLIKELY(NULL == node)) {
        return NULL;
    }

    if (boundary_tail == boundary) {
        while (NULL != node->next) {
            node = node->next;
        }
    } else {
        while (NULL != node->prev) {
            node = node->prev;
        }
    }

    return node;
}

static void cnt_nodes_internal(const dll_node* node, void* user_data)
{
    (void)node;
    size* ctr = user_data;
    *ctr += 1;
}

/* ------------------------------------------------------------ */
/* ----------------------- Api functions ---------------------- */
/* ------------------------------------------------------------ */

dll_status dll_node_create(dll_node* node, void* user_data)
{
    ERROR_IF(node, NULL, dll_status_iptr);
    node->prev = node->next = NULL;
    node->user_data = user_data;
    return dll_status_ok;
}

dll_status dll_destroy(dll_node* head, dll_node_decay_fn decay_fn)
{
#if DLL_HEAD_SANITY_CHECK
    if (!head_node_sanity_check(head)) {
        return dll_status_inv_node;
    }
#endif

    ERROR_IF(head, NULL, dll_status_iptr);
    ERROR_IF(decay_fn, NULL, dll_status_iptr);

    while (NULL != head) {
        dll_node* tmp = head->next;
        decay_fn(head);
        head = tmp;
    }
    return dll_status_ok;
}

dll_node* dll_node_insert_before(dll_node* act_node, dll_node* new_node, dll_status* status)
{
    bool invalid_args = (NULL == act_node) || (NULL == new_node);
    if (UNLIKELY(invalid_args)) {
        set_status(dll_status_iptr, status);
        return NULL;
    }

#if DLL_NEW_NODE_SANITY_CHECK
    if (UNLIKELY(!new_node_sanity_check(new_node))) {
        set_status(dll_status_inv_node, status);
        return NULL;
    }
#endif

    dll_node* prev_node = act_node->prev;

    new_node->prev = prev_node;
    new_node->next = act_node;
    act_node->prev = new_node;

    if (NULL != prev_node) {
        prev_node->next = new_node;
    }

    set_status(dll_status_ok, status);
    return dll_find_head(new_node);
}

dll_node* dll_node_insert_after(dll_node* act_node, dll_node* new_node, dll_status* status)
{
    bool invalid_args = (NULL == act_node) || (NULL == new_node);
    if (UNLIKELY(invalid_args)) {
        set_status(dll_status_iptr, status);
        return NULL;
    }

#if DLL_NEW_NODE_SANITY_CHECK
    if (UNLIKELY(!new_node_sanity_check(new_node))) {
        set_status(dll_status_inv_node, status);
        return NULL;
    }
#endif

    dll_node* next_node = act_node->next;

    new_node->prev = act_node;
    new_node->next = next_node;
    act_node->next = new_node;

    if (NULL != next_node) {
        next_node->prev = new_node;
    }

    set_status(dll_status_ok, status);
    return dll_find_head(new_node);
}

dll_node* dll_get_last_node(dll_node* head)
{
    return find_boundary(head, boundary_tail);
}

dll_node* dll_find_head(dll_node* node)
{
    return find_boundary(node, boundary_head);
}

dll_node* dll_node_delete_before(dll_node* act_node, dll_status* status, dll_node_decay_fn decay_fn)
{
    if (UNLIKELY(NULL == act_node)) {
        set_status(dll_status_iptr, status);
        return NULL;
    }

    set_status(dll_status_ok, status);
    dll_node* node_to_be_deleted = act_node->prev;

    /* Do nothing in case head was passed */
    if (NULL == node_to_be_deleted) {
        return act_node;
    }

    dll_node* to_be_deleted_prev = node_to_be_deleted->prev;
    act_node->prev = node_to_be_deleted->prev;
    if (NULL != to_be_deleted_prev) {
        to_be_deleted_prev->next = act_node;
    }

    /* Call decay function if needed */
    if (NULL != decay_fn) {
        decay_fn(node_to_be_deleted);
    }

    return dll_find_head(act_node);
}

dll_node* dll_node_delete_after(dll_node* act_node, dll_status* status, dll_node_decay_fn decay_fn)
{
    if (UNLIKELY(NULL == act_node)) {
        set_status(dll_status_iptr, status);
        return NULL;
    }

    set_status(dll_status_ok, status);
    dll_node* node_to_be_deleted = act_node->next;

    if (NULL == node_to_be_deleted) {
        return act_node;
    }

    dll_node* tbd_next = node_to_be_deleted->next;
    act_node->next = tbd_next;
    if (NULL != tbd_next) {
        tbd_next->prev = act_node;
    }

    /* Call decay function */
    if (NULL != decay_fn) {
        decay_fn(node_to_be_deleted);
    }

    return dll_find_head(act_node);
}

dll_node* dll_node_delete_begin(dll_node* head, dll_status* status, dll_node_decay_fn decay_fn)
{
    if (UNLIKELY(NULL == head)) {
        set_status(dll_status_iptr, status);
        return NULL;
    }

    set_status(dll_status_ok, status);
    dll_node* next = head->next;
    if (NULL == next) {
        if (NULL != decay_fn) {
            decay_fn(head);
        }
        return NULL;
    }

    return dll_node_delete_before(next, status, decay_fn);
}

dll_node* dll_node_delete_end(dll_node* head, dll_status* status, dll_node_decay_fn decay_fn)
{
    if (UNLIKELY(NULL == head)) {
        set_status(dll_status_iptr, status);
        return NULL;
    }

    set_status(dll_status_ok, status);
    dll_node* tail = dll_get_last_node(head);
    dll_node* prev = tail->prev;
    if (NULL == prev) {
        if (NULL != decay_fn) {
            decay_fn(head);
            return NULL;
        }
    }

    return dll_node_delete_after(prev, status, decay_fn);
}

void dll_traverse(const dll_node* head, dll_traverse_fn traverse_fn, void* user_data)
{
    if (UNLIKELY(NULL == head || NULL == traverse_fn)) {
        return;
    }

    while (NULL != head) {
        traverse_fn(head, user_data);
        head = head->next;
    }
}

size dll_node_count(const dll_node* head)
{
    size cnt = 0;
    dll_traverse(head, cnt_nodes_internal, &cnt);
    return cnt;
}

dll_node* dll_node_find(dll_node* head, dll_node_cmp_fn compare_fn)
{
    ERROR_IF(compare_fn, NULL, NULL);

    while (NULL != head) {
        if (compare_fn(dll_get_user_data(head))) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

dll_node* dll_node_insert_begin(dll_node* head, dll_node* new_node, dll_status* status)
{
    return dll_node_insert_before(head, new_node, status);
}

dll_node* dll_node_insert_end(dll_node* head, dll_node* new_node, dll_status* status)
{
    return dll_node_insert_after(dll_get_last_node(head), new_node, status);
}
