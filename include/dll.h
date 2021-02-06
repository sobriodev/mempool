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
#define DLL_API_VERSION_MINOR 1
/** Revision version */
#define DLL_API_VERSION_REVISION 0

/* ------------------------------------------------------------ */
/* ------------------------ Data types ------------------------ */
/* ------------------------------------------------------------ */

/** Return codes */
typedef enum dll_status_
{
    dll_status_ok = 0, /**< OK */
    dll_status_iptr /**< Unexpected NULL pointer */
} dll_status;

/** dll_node struct */
typedef struct dll_node dll_node;

/** Node decay function type */
typedef void (*dll_node_decay_fn)(const dll_node* node);

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
 * @param decay_fn Pointer to optional function called on node decay.
 * @return dll_status_iptr when NULL was passed instead of a valid pointer, dll_status_ok otherwise.
 */
dll_status dll_create(dll_node* head, void* user_data, dll_node_decay_fn decay_fn);

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
