#include "dll.h"

/* ------------------------------------------------------------ */
/* ---------------------- Private functions ------------------- */
/* ------------------------------------------------------------ */

static void set_status(dll_status status, dll_status* out)
{
    if (LIKELY(NULL != out)) {
        *out = status;
    }
}

/* ------------------------------------------------------------ */
/* ----------------------- Api functions ---------------------- */
/* ------------------------------------------------------------ */

dll_status dll_create(dll_node* head, void* user_data)
{
    ERROR_IF(head, NULL, dll_status_iptr);
    head->prev = NULL;
    head->next = NULL;
    head->user_data = user_data;
    return dll_status_ok;
}

dll_status dll_destroy(dll_node* head, dll_node_decay_fn decay_fn)
{
    ERROR_IF(head, NULL, dll_status_iptr);
    ERROR_IF(decay_fn, NULL, dll_status_iptr);

    dll_node* node = head;
    while(NULL != node) {
        dll_node* tmp = node->next;
        decay_fn(node);
        node = tmp;
    }
    return dll_status_ok;
}

dll_node* dll_node_insert_begin(dll_node* head, dll_node* new_node, dll_status* status)
{
    if (UNLIKELY(NULL == new_node)) {
        set_status(dll_status_iptr, status);
        return NULL;
    }

    new_node->next = head;
    if (NULL != head) {
        head->prev = new_node;
    }
    set_status(dll_status_ok, status);
    return new_node;
}

