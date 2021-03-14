#include "mempool.h"
#include "bit.h"
#include "dll.h"

/* ------------------------------------------------------------ */
/* ---------------------- Private data types ------------------ */
/* ------------------------------------------------------------ */

typedef struct room_header_
{
    u32 size; /* 4B */
    u8 active; /* 5B */
    u8 reserved[3]; /* 8B */
} room_header;

/* Struct used in debug_traverse_imp() function */
typedef struct dbg_traverse_user_data_
{
    u32 next_idx;
    mempool_debug_info* dbg_info;
} dbg_traverse_user_data;

/* ------------------------------------------------------------ */
/* ----------------------- Private functions ------------------ */
/* ------------------------------------------------------------ */

/* Check if a number is power of two */
static inline bool is_power_of_two(u32 number)
{
    return (0 != number) && (!(number & (number - 1)));
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
}

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

u32 mempool_calc_hdr_size()
{
    return sizeof(dll_node) + sizeof(room_header);
}

u32 mempool_partitions_used(const mempool_instance* pool)
{
    ERROR_IF(pool, NULL, 0);
    return dll_node_count((const dll_node*)pool->base_addr);
}

u32 mempool_decode_debug_info(const mempool_instance* pool, mempool_debug_info* dbg_info)
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
