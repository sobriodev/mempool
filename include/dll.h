#ifndef MEMPOOL_DLL_H
#define MEMPOOL_DLL_H

#include "type.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------ */
/* -------------------------- Macros -------------------------- */
/* ------------------------------------------------------------ */

/** Major version */
#define DLL_API_VERSION_MAJOR 0
/** Minor version */
#define DLL_API_VERSION_MINOR 2
/** Revision version */
#define DLL_API_VERSION_REVISION 0

/* ------------------------------------------------------------ */
/* ------------------------ Data types ------------------------ */
/* ------------------------------------------------------------ */

/** Return codes */
typedef enum dll_status_
{
    dll_status_ok = 0, /**< OK */
    dll_status_nok, /**< Generic error code */
    dll_status_iptr, /**< Unexpected NULL pointer */
    dll_status_inv_node /**< Invalid node */
} dll_status;

/** dll_node struct */
typedef struct dll_node dll_node;

/** Node decay function type */
typedef void (*dll_node_decay_fn)(dll_node* node);

/** Traverse function type */
typedef void (*dll_traverse_fn)(const dll_node* node, void* user_data);

/** Compare node function type */
typedef bool (*dll_node_cmp_fn)(const void* user_data);

/** Node implementation */
struct dll_node
{
    dll_node* prev; /**< Previous node */
    dll_node* next; /**< Next node */
    void* user_data; /**< User data */
};

/* ------------------------------------------------------------ */
/* ----------------------- Api functions ---------------------- */
/* ------------------------------------------------------------ */

/**
 * Initialize doubly-linked list instance.
 *
 * The function set previous and next nodes to NULL.
 *
 * @param head Pointer to memory where dll is stored.
 * @param user_data Pointer to user data.
 * @return dll_status_iptr when NULL was passed instead of a valid pointer, dll_status_ok otherwise.
 */
dll_status dll_create(dll_node* head, void* user_data);

/**
 * Destroy doubly-linked list.
 *
 * The call to this function is optional and has to be performed only when deleting a node
 * requires specific behaviour (e.g. freeing memory when the nodes were allocated on heap).
 *
 * @param head Pointer to a head node.
 * @param decay_fn Decay function pointer.
 * @return The function returns dll_status_iptr when NULL pointer was passed in place of head node or decay function,
 *         dll_status_ok otherwise.
 */
dll_status dll_destroy(dll_node* head, dll_node_decay_fn decay_fn);

/**
 * Insert a node before another existing one.
 *
 * This function performs sanity check on the new node before actual operation is executed, namely pointers to the
 * prev and next node have to be NULL-ed. Inserting whole lists is not supported by this function.
 *
 * @param act_node Pointer to an existing node. Cannot be NULL.
 * @param new_node Pointer to a node to be inserted. Cannot be NULL.
 * @param status Memory address where status of the operation will be stored. Can be NULL. Supported values are:
 *               - dll_status_iptr in case NULL was passed instead of a valid pointer
 *               - dll_status_inv_node in case sanity check has failed
 *               - dll_status_ok on success
 * @return Pointer to a new head of the list. It may change or not, depending on a place where the new node was
 *         inserted.
 */
dll_node* dll_node_insert_before(dll_node* act_node, dll_node* new_node, dll_status* status);

/**
 * Insert a node after another existing one.
 *
 * This function performs sanity check on the new node before actual operation is executed, namely pointers to the
 * prev and next node have to be NULL-ed. Inserting whole lists is not supported by this function.
 *
 * @param act_node Pointer to an existing node. Cannot be NULL.
 * @param new_node Pointer to a node to be inserted. Cannot be NULL.
 * @param status Memory address where status of the operation will be stored. Can be NULL. Supported values are:
 *               - dll_status_iptr in case NULL was passed instead of a valid pointer
 *               - dll_status_inv_node in case sanity check has failed
 *               - dll_status_ok on success
 * @return Pointer to the head of the list. The function never changes it since the new node is always inserted after
 *         an existing one.
 */
dll_node* dll_node_insert_after(dll_node* act_node, dll_node* new_node, dll_status* status);

/**
 * Find the head of a list.
 *
 * @param node Pointer to any node that forms the list.
 * @return Pointer to the head or NULL in case NULL was passed to the function.
 */
dll_node* dll_find_head(dll_node* node);

/**
 * Find the tail (last node) of a list.
 *
 * @param head Pointer to any node that forms the list.
 * @return Pointer to the tail or NULL when NULL was passed to the function.
 */
dll_node* dll_get_last_node(dll_node* head);

/**
 * Insert a node at the end of the list.
 *
 * TODO perform sanity check on the head also (ENH/4 ticket on github)
 *
 * @param head Pointer to a head node. Cannot be NULL.
 * @param new_node Pointer to a new node. Cannot be NULL.
 * @param status Memory address where status of the operation will be stored. Can be NULL. Possible values are:
 *               - dll_status_iptr in case NULL was passed instead of a valid pointer
 *               - dll_status_inv_node in case sanity check has failed
 *               - dll_status_ok on success
 * @return Pointer to the head of the list. The function never changes it since the new node is always inserted after
 *         an existing one.
 */
static inline dll_node* dll_node_insert_end(dll_node* head, dll_node* new_node, dll_status* status)
{
    return dll_node_insert_after(dll_get_last_node(head), new_node, status);
}

/**
 * Insert a node at the beginning of the list.
 *
 * TODO perform sanity check on the tail also (ENH/4 ticket on github)
 *
 * @param head Pointer to a head node. Cannot be NULL.
 * @param new_node Pointer to a new node. Cannot be NULL.
 * @param status Memory address where status of the operation will be stored. Can be NULL. Possible values are:
 *               - dll_status_iptr in case NULL was passed instead of a valid pointer
 *               - dll_status_inv_node in case sanity check has failed
 *               - dll_status_ok on success
 * @return Pointer to the the new head node. It will be the the same pointer as new_node after the function succeeded.
 */
static inline dll_node* dll_node_insert_begin(dll_node* head, dll_node* new_node, dll_status* status)
{
    return dll_node_insert_before(head, new_node, status);
}

/**
 * Delete predecessor of an existing node.
 *
 * The function does nothing when the list consists of single node only.
 *
 * @param act_node Pointer to an existing node. Cannot be NULL.
 * @param status Memory address of the variable where the status code will be stored. Can be NULL. The function returns:
 *               - dll_status_iptr in case NULL was passed instead of a valid node
 *               - dll_status_ok otherwise
 * @param decay_fn Optional function pointer called on node decay.
 * @return Pointer to a new head node of the list.
 */
dll_node* dll_node_delete_before(dll_node* act_node, dll_status* status, dll_node_decay_fn decay_fn);

/**
 * Delete successor of an existing node.
 *
 * The function does nothing in case the list consists of single node only.
 *
 * @param act_node Pointer to an actual node. Cannot be NULL.
 * @param status Pointer to a variable where status of the operation will be stored. Can be NULL. The function returns:
 *               - dll_status_iptr in case NULL was passed instead of a valid node
 *               - dll_status_ok on success
 * @param decay_fn Pointer to an optional function that will be called when a node is destroyed.
 * @return Pointer to a new head node.
 */
dll_node* dll_node_delete_after(dll_node* act_node, dll_status* status, dll_node_decay_fn decay_fn);

/**
 * Delete first node of the list.
 *
 * TODO maybe sanity check?
 *
 * In case the list consists of single node only it will be destroyed and NULL is returned afterwards.
 *
 * @param head A pointer to the head node of the list. Cannot be NULL.
 * @param status A pointer to variable where the status will be stored. Can be NULL. Valid results are:
 *               - dll_status_iptr when NULL was passed instead of a valid head pointer
 *               - dll_status_ok on success
 * @param decay_fn Optional pointer to a function that will be called on node decay.
 * @return A pointer to the new head node of the list.
 */
dll_node* dll_node_delete_begin(dll_node* head, dll_status* status, dll_node_decay_fn decay_fn);

/**
 * Delete last node of the list.
 *
 * TODO maybe sanity check?
 *
 * In case the list consists of single node only it will be destroyed and NULL is returned afterwards.
 *
 * @param head A pointer to the head node of the list. Cannot be NULL.
 * @param status A pointer to variable where the status will be stored. Can be NULL. Valid results are:
 *               - dll_status_iptr when NULL was passed instead of a valid head pointer
 *               - dll_status_ok on success
 * @param decay_fn Optional pointer to a function that will be called on node decay.
 * @return A pointer to the new head node of the list.
 */
dll_node* dll_node_delete_end(dll_node* head, dll_status* status, dll_node_decay_fn decay_fn);

/**
 * Traverse doubly-linked list with specific user function.
 *
 * There is no possibility to modify node's data since each node is passed as a const pointer.
 * The function does nothing in case list is empty (NULL passed) or pointer to traverse function is NULL.
 *
 * @param head A pointer to head node.
 * @param traverse_fn A pointer to user specific function.
 * @param user_data Optional pointer to user data shared across all function calls.
 */
void dll_traverse(const dll_node* head, dll_traverse_fn traverse_fn, void* user_data);

/**
 * Count the number of nodes in a doubly-linked list.
 *
 * @param head A pointer to head node of the list. Can be NULL. In this case zero is returned.
 * @return The number of nodes in the list.
 */
size dll_node_count(const dll_node* head);

/**
 * Find address of a node whose user data equals to searched one.
 *
 * The function uses custom compare function passed as an argument. In case NULL was passed NULL is returned.
 * The user data passed to the compare function is immutable.
 *
 * @param head A pointer to the head node. Can be NULL.
 * @param compare_fn A pointer to an user specific compare function.
 * @return An address of the searched node or NULL if nothing was found.
 */
dll_node* dll_node_find(dll_node* head, dll_node_cmp_fn compare_fn);

/**
 * Get node's user data.
 *
 * @param node Pointer to dll node. Cannot be NULL.
 * @return Pointer to user data.
 */
static inline void* dll_get_user_data(const dll_node* node)
{
    return node->user_data;
}

/**
 * Get previous node.
 *
 * @param node Pointer to dll node. Cannot be NULL.
 * @return Previous node.
 */
static inline dll_node* dll_get_prev_node(const dll_node* node)
{
    return node->prev;
}

/**
 * Get next node.
 *
 * @param node Pointer to dll node. Cannot be NULL.
 * @return Next node.
 */
static inline dll_node* dll_get_next_node(const dll_node* node)
{
    return node->next;
}

#ifdef __cplusplus
}
#endif

#endif //MEMPOOL_DLL_H
