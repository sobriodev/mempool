#include "dll.h"

/* ------------------------------------------------------------ */
/* ----------------------- Api functions ---------------------- */
/* ------------------------------------------------------------ */

dll_status dll_create(dll_node* head, void* user_data, dll_node_decay_fn decay_fn)
{
    ERROR_IF(head, NULL, dll_status_iptr);
    head->prev = NULL;
    head->next = NULL;
    head->user_data = user_data;
    head->decay_fn = decay_fn;
    return dll_status_ok;
}