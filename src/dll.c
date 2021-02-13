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

static inline bool new_node_sanity_check(dll_node* node)
{
    return (NULL == node->prev) && (NULL == node->next);
}

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

/* ------------------------------------------------------------ */
/* ----------------------- Api functions ---------------------- */
/* ------------------------------------------------------------ */

dll_status dll_create(dll_node* head, void* user_data)
{
    ERROR_IF(head, NULL, dll_status_iptr);
    head->prev = head->next = NULL;
    head->user_data = user_data;
    return dll_status_ok;
}

dll_status dll_destroy(dll_node* head, dll_node_decay_fn decay_fn)
{
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

    if (UNLIKELY(!new_node_sanity_check(new_node))) {
        set_status(dll_status_inv_node, status);
        return NULL;
    }

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

    if (UNLIKELY(!new_node_sanity_check(new_node))) {
        set_status(dll_status_inv_node, status);
        return NULL;
    }

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
