#ifndef MEMPOOL_DLL_H
#define MEMPOOL_DLL_H

#include "type.h"

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
    dll_status_ok = 0 /**< OK */
} dll_status;

/** dll_node struct */
typedef struct dll_node dll_node;

/** Node implementation */
struct dll_node
{
    dll_node* prev; /**< Previous node */
    dll_node* next; /**< Next node */
    void* buffer; /**< User data */
};

#ifdef __cplusplus
}
#endif

#endif //MEMPOOL_DLL_H
