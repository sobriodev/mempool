#include "dll.h"

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
