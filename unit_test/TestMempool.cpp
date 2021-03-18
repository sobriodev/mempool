#include "TestRunner.h"
#include "mempool.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

TEST_GROUP(Mempool)
{
    /* Buffer used for general testing */
    static const size BUFFER_1K_SIZE = 1024;
    i8* buffer1K = nullptr; /* Allocation and freeing handled in setup() and teardown() functions respectively. */

    void setup() override
    {
        buffer1K = new i8[BUFFER_1K_SIZE];
    }

    void teardown() override
    {
        delete[] buffer1K;
    }

    auto initMempoolWith1KBuffer() const
    {
        mempool_instance inst;
        inst.base_addr = buffer1K;
        inst.size = BUFFER_1K_SIZE;
        CHECK_EQUAL(mempool_status_ok, mempool_init(&inst));
        return inst;
    }
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(Mempool, mempool_init__NullCases)
{
    CHECK_EQUAL(mempool_status_nullptr, mempool_init(nullptr));

    mempool_instance pool;
    pool.base_addr = nullptr;
    pool.size = BUFFER_1K_SIZE;
    CHECK_EQUAL(mempool_status_nullptr, mempool_init(&pool));
}

TEST(Mempool, mempool_init__SizeNotPowerOf2__ErrorGenerated)
{
    const size invalidSizeTbl[] = {
        0, /* Zero-case should also be handled */
        3, 10, 127, 257, 1025, 2000, 6718291, UINT32_MAX
    };

    mempool_instance pool;
    pool.base_addr = buffer1K;
    for (const auto& size: invalidSizeTbl) {
        pool.size = size; /* Set new size and call init function */
        CHECK_EQUAL(mempool_status_size_err, mempool_init(&pool));
    }
}

TEST(Mempool, mempool_init__BufferSizeTooSmall__ErrorReturned)
{
    mempool_instance pool;
    pool.base_addr = buffer1K;
    pool.size = 1;
    CHECK_EQUAL(mempool_status_out_of_memory, mempool_init(&pool));
}

TEST(Mempool, mempool_init__ValidParams__Success)
{
    auto pool = initMempoolWith1KBuffer();
    auto numOfRooms = mempool_partitions_used(&pool);
    CHECK_EQUAL(1, numOfRooms);

    mempool_debug_info dbgInfo[numOfRooms];
    CHECK_EQUAL(1, mempool_decode_debug_info(&pool, dbgInfo));

    mempool_debug_info* first_room_dbg = &dbgInfo[0];
    CHECK_TRUE(first_room_dbg->is_first);
    CHECK_TRUE(first_room_dbg->is_last);
    CHECK_FALSE(first_room_dbg->room_occupied);
    CHECK_EQUAL(BUFFER_1K_SIZE, first_room_dbg->room_size);
    CHECK_EQUAL(BUFFER_1K_SIZE - mempool_calc_hdr_size(), first_room_dbg->usable_size);
}

TEST(Mempool, mempool_partitions_used__NullPassed__ZeroReturned)
{
    CHECK_EQUAL(0, mempool_partitions_used(nullptr));
}

TEST(Mempool, mempool_decode_debug_info__InvalidPointerPassed__ZeroReturnedWhen)
{
    auto pool = initMempoolWith1KBuffer();
    mempool_debug_info dbgInfo;
    CHECK_EQUAL(0, mempool_decode_debug_info(nullptr, &dbgInfo));
    CHECK_EQUAL(0, mempool_decode_debug_info(&pool, nullptr));
}