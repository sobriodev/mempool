#ifndef MEMPOOL_MEMPOOL_H
#define MEMPOOL_MEMPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "common.h"

/* ------------------------------------------------------------ */
/* ---------------------------- Macros ------------------------ */
/* ------------------------------------------------------------ */

/** Major version */
#define MEMPOOL_API_VERSION_MAJOR 0
/** Minor version */
#define MEMPOOL_API_VERSION_MINOR   1
/** Revision version */
#define MEMPOOL_API_VERSION_REVISION 0

/* ------------------------------------------------------------ */
/* -------------------------- Data types ---------------------- */
/* ------------------------------------------------------------ */

/** Return codes for API functions */
typedef enum mempool_status_
{
    mempool_status_ok, /**< Success */
    mempool_status_nok, /**< General error code */
    mempool_status_size_err, /**< Pool size is not a power of two */
    mempool_status_out_of_memory, /**< Out of memory */
    mempool_status_nullptr /**< Unexpected NULL pointer */
} mempool_status;

/** Mempool instance holding all information */
typedef struct mempool_instance_
{
    i8* base_addr; /**< Base address of the pool buffer */
    u32 size; /**< Size of the pool buffer */
} mempool_instance;

/** Mempool debug info structure. May be used for testing purposes */
typedef struct mempool_debug_info_
{
    bool is_first; /**< True if the partition does not have predecessor */
    bool is_last; /**< True if the partition does not have successor */
    bool room_occupied; /**< True if the partition is occupied */
    u32 room_size; /**< Size of the partition */
    u32 usable_size; /**< Size available for the user */
} mempool_debug_info;

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

/**
 * Initialize mempool instance.
 *
 * The function has to be invoked before any other API functions. It creates a single memory partition that occupies all
 * available space based on parameters inside 'pool' variable. Mempool module assumes that memory buffer was allocated
 * prior to calling this API function and it will be freed outside this module. Memory buffer has to follow below rules:
 *  1. Its size must be a power of two
 *  2. It has to be large enough to contain partition header + 1 extra byte - use mempool_calc_hdr_size() to calculate
 *     header length
 *
 * @param pool Pointer to a struct containing pool properties. The struct has to be initialized with valid values.
 * @return Status of the operation:
 *         - mempool_status_nullptr in case NULL was passed instead of a valid pointer
 *         - mempool_status_size_err in case size of the memory buffer is not a power of two
 *         - mempool_status_out_of_memory when the buffer is to small to allocate first partition
 *         - mempool_status_nok in case of general failure that cannot be handled directly in the function
 *         - mempool_status_ok on success
 */
mempool_status mempool_init(mempool_instance* pool);

/**
 * Calculate how many bytes are needed to store partition's metadata.
 *
 * This chunk of memory is excluded from general usage and it is hidden from the user (no need to manually move N bytes
 * forward to get usable memory pointer).
 *
 * @return The number of bytes.
 */
u32 mempool_calc_hdr_size();

/**
 * Calculate how many partitions are available.
 *
 * The function counts both occupied and not occupied ones. The pool has to be initialized at the time this function is
 * called.
 *
 * @param pool Pointer to a pool instance.
 * @return The number of partitions used or zero when NULL was passed.
 */
u32 mempool_partitions_used(const mempool_instance* pool);

/**
 * Decode pool's debug data.
 *
 * The function may be called in a testing environment. None of the API functions use it directly. It fills the
 * 'dbg_info' vector with 'mempool_debug_info' rows. Each row contains information about single partition created.
 * The vector has to be large enough to accommodate all information. Its size can be calculated using
 * mempool_partitions_used() API call. The function returns 0 if either 'pool' or 'dbg_info' pointers is NULL.
 *
 * @param pool Pointer to a pool instance.
 * @param dbg_info Pointer to a debug vector.
 * @return The number of rows written.
 */
u32 mempool_decode_debug_info(const mempool_instance* pool, mempool_debug_info* dbg_info);

#ifdef __cplusplus
}
#endif

#endif //MEMPOOL_MEMPOOL_H
