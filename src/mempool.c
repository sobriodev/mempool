#include "mempool.h"
#include "bit.h"
#include "dll.h"

/* Force the module to be compiled under specific architecture to prevent from unaligned memory accesses */
#if (MEMPOOL_CPU_ARCH != 16) && (MEMPOOL_CPU_ARCH != 32) && (MEMPOOL_CPU_ARCH != 64)
#error "Mempool: Not supported CPU architecture! (Set MEMPOOL_CPU_ARCH macro)"
#endif

/* Enable sanity checks only on 32/64-bit architectures */
#if MEMPOOL_CPU_ARCH == 16
#if MEMPOOL_SANITY_CHECK
#error "Mempool: Sanity checks not available on 16-bit architecture!"
#endif
#endif

/* ------------------------------------------------------------ */
/* ---------------------- Private data types ------------------ */
/* ------------------------------------------------------------ */

#if MEMPOOL_SANITY_CHECK
/* Magic number used in sanity checks */
#define MAGIC_NUMBER 0xFEED
#endif

/* ------------------------------------------------------------ */
/* ---------------------- Private data types ------------------ */
/* ------------------------------------------------------------ */

typedef struct room_header_
{
    size size;
    u8 active;
#if MEMPOOL_CPU_ARCH == 16
    u8 _reserved[1];
#elif MEMPOOL_CPU_ARCH == 32
#if MEMPOOL_SANITY_CHECK
    u16 magic;
    u8 _reserved[1];
#else
    u8 _reserved[3];
#endif
#elif MEMPOOL_CPU_ARCH == 64
#if MEMPOOL_SANITY_CHECK
    u16 magic;
    u8 _reserved[5];
#else
    u8 _reserved[7];
#endif
#endif
} room_header;

/* Struct used in debug_traverse_imp() function */
typedef struct dbg_traverse_user_data_
{
    size next_idx;
    mempool_debug_info* dbg_info;
} dbg_traverse_user_data;

/* For searching buddy partitions */
typedef enum buddy_pos_
{
    buddy_pos_left,
    buddy_pos_right
} buddy_pos;

/* Status codes for merge function */
typedef enum merge_status_
{
    merge_status_internal_err,
    merge_status_merged,
    merge_status_not_merged
} merge_status;

/* ------------------------------------------------------------ */
/* ----------------------- Private functions ------------------ */
/* ------------------------------------------------------------ */

/* Check if a number is power of two */
static inline bool is_power_of_two(size number)
{
    return (0 != number) && (!(number & (number - 1)));
}

/* Round up a number to the next power of two */
static size round_pow_two(size v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

/* Function used in mempool_decode_debug_info() to decode debug data */
static void debug_traverse_imp(const dll_node* node, void* user_data)
{
    /* Calculate descriptor address */
    room_header* header = (room_header*)dll_get_user_data(node);
    dbg_traverse_user_data* dbg_data = user_data;
    mempool_debug_info* dbg_tbl_row = &dbg_data->dbg_info[dbg_data->next_idx++];
    dbg_tbl_row->is_first = (NULL == dll_get_prev_node(node));
    dbg_tbl_row->is_last = (NULL == dll_get_next_node(node));
    dbg_tbl_row->room_size = header->size;
    dbg_tbl_row->room_occupied = header->active;
    dbg_tbl_row->usable_size = header->size - mempool_calc_hdr_size();
    dbg_tbl_row->base_addr = node;
    dbg_tbl_row->usable_space_addr = (const char*)(node) + mempool_calc_hdr_size();
}

/* Implementation of dll find function */
static bool find_partition_impl(const dll_node* node, void* user_data)
{
    const room_header* hdr = dll_get_user_data(node);
    size* total_len = user_data;
    return hdr->size >= *total_len && !hdr->active;
}

/* Split partition */
static bool split_partition(dll_node* partition)
{
    room_header* hdr = dll_get_user_data(partition);
    size new_len = hdr->size / 2;

    /* Create buddy partition */
    dll_node* new_buddy = (dll_node*)((char*)partition + new_len);
    room_header* new_buddy_hdr = (room_header*)((char*)new_buddy + sizeof(dll_node));
    if (UNLIKELY(dll_status_ok != dll_node_create(new_buddy, new_buddy_hdr))) {
        return false;
    }

    dll_set_user_data(new_buddy, new_buddy_hdr);

    /* Link partitions */
    dll_status status = dll_status_nok;
    dll_node_insert_after(partition, new_buddy, &status);
    if (dll_status_ok != status) {
        return false;
    }

    /* Update headers */
    hdr->size = new_len;
    new_buddy_hdr->size = new_len;
    new_buddy_hdr->active = false;

    return true;
}

/* Search for buddy partition */
static dll_node* get_buddy(dll_node* partition, size part_size, buddy_pos buddy_pos)
{
    dll_node* buddy = (buddy_pos_left == buddy_pos) ? dll_get_prev_node(partition) : dll_get_next_node(partition);
    if (NULL != buddy) {
        room_header* buddy_hdr = dll_get_user_data(buddy);
        if (part_size == buddy_hdr->size) {
            return buddy;
        }
    }
    return NULL;
}

static merge_status merge_partitions(dll_node* partition)
{
    room_header* hdr = dll_get_user_data(partition);

    dll_node* left = get_buddy(partition, hdr->size, buddy_pos_left);
    room_header* left_hdr;

    if (NULL != left) {
        /* There is a buddy on the left */
        left_hdr = dll_get_user_data(left);
        if (left_hdr->active) {
            return merge_status_not_merged; /* Occupied - do nothing */
        }
    } else {
        dll_node* tmp = get_buddy(partition, hdr->size, buddy_pos_right);
        if (NULL != tmp) {
            /* There is a buddy on the right */
            room_header* tmp_hdr = dll_get_user_data(tmp);
            if (tmp_hdr->active) {
                return merge_status_not_merged; /* Occupied - do nothing */
            }
            left = partition;
            left_hdr = hdr;
        } else {
            /* The partition does not have a buddy - do nothing */
            return merge_status_not_merged;
        }
    }

    /* Merge partitions */
    left_hdr->size *= 2;
    dll_status stat;
    dll_node_delete_after(left, &stat, NULL);
    return (dll_status_ok == stat) ? merge_status_merged : merge_status_internal_err;
}

#if MEMPOOL_SANITY_CHECK
static inline bool partition_sanity_check(const room_header* hdr)
{
    return (MAGIC_NUMBER == hdr->magic);
}
#endif

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

mempool_status mempool_init(mempool_instance* pool)
{
    ERROR_IF(pool, NULL, mempool_status_nullptr);
    ERROR_IF(pool->base_addr, NULL, mempool_status_nullptr);

    /* Return error code when wrong size was passed */
    ERROR_IF(is_power_of_two(pool->size), false, mempool_status_size_err);

    /* Check if there is enough space to create first room */
    if (UNLIKELY(pool->size <= mempool_calc_hdr_size())) {
        return mempool_status_out_of_memory;
    }

    /* Allocate first room that occupies all available space */
    dll_node* node = (dll_node*)pool->base_addr;
    room_header* header = (room_header*)(pool->base_addr + sizeof(dll_node));
    if (UNLIKELY(dll_status_ok != dll_node_create(node, header))) {
        return mempool_status_nok;
    }
    header->size = pool->size;
    header->active = false;

    return mempool_status_ok;
}

size mempool_calc_hdr_size()
{
    return sizeof(dll_node) + sizeof(room_header);
}

size mempool_partitions_used(const mempool_instance* pool)
{
    ERROR_IF(pool, NULL, 0);
    return dll_node_count((const dll_node*)pool->base_addr);
}

size mempool_decode_debug_info(const mempool_instance* pool, mempool_debug_info* dbg_info)
{
    ERROR_IF(pool, NULL, 0);
    ERROR_IF(dbg_info, NULL, 0);

    /* Struct instance passed as user data */
    dbg_traverse_user_data dbg_user_data;
    dbg_user_data.dbg_info = dbg_info;
    dbg_user_data.next_idx = 0;

    const dll_node* head = (const dll_node*)pool->base_addr;
    dll_traverse(head, debug_traverse_imp, &dbg_user_data);
    return dbg_user_data.next_idx;
}

mempool_status mempool_claim_memory(const mempool_instance* pool, size len, void** dst)
{
    ERROR_IF(pool, NULL, mempool_status_nullptr);
    ERROR_IF(len, 0, mempool_status_size_err);
    ERROR_IF(dst, NULL, mempool_status_nullptr);

    /* Round up the size if needed */
    size total_len = round_pow_two(len + mempool_calc_hdr_size());
    dll_node* lst = (dll_node*)pool->base_addr;

    dll_node* partition = dll_node_find(lst, find_partition_impl, &total_len);
    if (NULL == partition) {
        return mempool_status_out_of_memory;
    }
    room_header* hdr = dll_get_user_data(partition);

    /* Split partitions if needed */
    while (hdr->size > total_len) {
        if (!split_partition(partition)) {
            return mempool_status_nok;
        }
    }

    hdr->active = true;
    *dst = (char*)partition + mempool_calc_hdr_size();

    return mempool_status_ok;
}

mempool_status mempool_free_memory(const mempool_instance* pool, void* memory)
{
    ERROR_IF(pool, NULL, mempool_status_nullptr);
    ERROR_IF(memory, NULL, mempool_status_nullptr);

    dll_node* partition = (dll_node*)((char*)memory - mempool_calc_hdr_size());
    room_header* hdr = dll_get_user_data(partition);

#ifdef MEMPOOL_SANITY_CHECK
    ERROR_IF(partition_sanity_check(hdr), false, mempool_status_inv_memory);
#endif

    /* Throw an error in case partition is not active */
    ERROR_IF(hdr->active, false, mempool_status_inv_memory);

    /* Clear active flag to reuse the partition in the future */
    hdr->active = false;

    /* Merge partitions as long as possible */
    merge_status merge_stat;
    do {
        merge_stat = merge_partitions(partition);
    } while (merge_status_merged == merge_stat);
    ERROR_IF(merge_stat, merge_status_internal_err, mempool_status_nok);

    return mempool_status_ok;
}
