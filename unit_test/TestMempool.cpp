#include "TestRunner.h"
#include "mempool.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

TEST_GROUP(Mempool)
{
    /* Buffer used for general testing */
    static const size BUFFER_1K_SIZE = 1024;
    char* buffer1K = nullptr; /* Allocation and freeing handled in setup() and teardown() functions respectively */

    void setup() override
    {
        buffer1K = new char[BUFFER_1K_SIZE];
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

    /* The pool has to be initialized */
    static auto claimMemory(const mempool_instance* pool, size len)
    {
        void* dst = nullptr;
        auto status = mempool_claim_memory(pool, len, &dst);
        CHECK_EQUAL(mempool_status_ok, status);
        CHECK(nullptr != dst);
        return dst;
    }

    static void testDbgData(const mempool_instance* pool, const mempool_debug_info* expectedVct, size expectedLen)
    {
        auto partitions = mempool_partitions_used(pool);
        CHECK_EQUAL(expectedLen, partitions);
        mempool_debug_info dbgInfo[partitions];
        auto items = mempool_decode_debug_info(pool, &dbgInfo[0]);
        CHECK_EQUAL(partitions, items);

        for (size i = 0; i < expectedLen; ++i) {
            const auto expectedInfo = &expectedVct[i];
            const auto actualInfo = &dbgInfo[i];
            CHECK_EQUAL(expectedInfo->is_first, actualInfo->is_first);
            CHECK_EQUAL(expectedInfo->is_last, actualInfo->is_last);
            CHECK_EQUAL(expectedInfo->room_occupied, actualInfo->room_occupied);
            CHECK_EQUAL(expectedInfo->room_size, actualInfo->room_size);
            CHECK_EQUAL(expectedInfo->usable_size, actualInfo->usable_size);
            POINTERS_EQUAL(expectedInfo->base_addr, actualInfo->base_addr);
            POINTERS_EQUAL(expectedInfo->usable_space_addr, actualInfo->usable_space_addr);
        }
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

TEST(Mempool, mempool_claim_memory__NullCases)
{
    void* dst = nullptr;
    CHECK_EQUAL(mempool_status_nullptr, mempool_claim_memory(nullptr, 10, &dst));
    auto pool = initMempoolWith1KBuffer();
    CHECK_EQUAL(mempool_status_nullptr, mempool_claim_memory(&pool, 5, nullptr));
}

TEST(Mempool, mempool_claim_memory__AfterInit__RequestZeroBytes__Error)
{
    auto pool = initMempoolWith1KBuffer();
    void* dst = nullptr;
    CHECK_EQUAL(mempool_status_size_err, mempool_claim_memory(&pool, 0, &dst));
}

TEST(Mempool, mempool_claim_memory__AfterInit__RequestTooMuchMemory__ErrorReturned)
{
    const auto claimSize = BUFFER_1K_SIZE;
    auto pool = initMempoolWith1KBuffer();
    void* dst = nullptr;
    CHECK_EQUAL(mempool_status_out_of_memory, mempool_claim_memory(&pool, claimSize, &dst));
    POINTERS_EQUAL(nullptr, dst);
}

TEST(Mempool, mempool_claim_memory__AfterInit__ClaimAllMemory__Success)
{
    const auto claimSize = BUFFER_1K_SIZE - mempool_calc_hdr_size();
    auto pool = initMempoolWith1KBuffer();
    auto dst = claimMemory(&pool, claimSize);
    CHECK_EQUAL(1, mempool_partitions_used(&pool));

    mempool_debug_info expectedDbgInfo;
    expectedDbgInfo.usable_space_addr = dst;
    expectedDbgInfo.base_addr = pool.base_addr;
    expectedDbgInfo.usable_size = claimSize;
    expectedDbgInfo.room_size = BUFFER_1K_SIZE;
    expectedDbgInfo.room_occupied = true;
    expectedDbgInfo.is_last = true;
    expectedDbgInfo.is_first = true;

    testDbgData(&pool, &expectedDbgInfo, 1);
}

TEST(Mempool, mempool_claim_memory__AfterInit__ClaimMemory__SizeRoundedUp)
{
    /* Subtract one to check if the size will be rounded up */
    const auto claimSize = BUFFER_1K_SIZE - mempool_calc_hdr_size() - 1;
    auto pool = initMempoolWith1KBuffer();
    auto dst = claimMemory(&pool, claimSize);
    CHECK_EQUAL(1, mempool_partitions_used(&pool));

    mempool_debug_info expectedDbgInfo;
    expectedDbgInfo.usable_space_addr = dst;
    expectedDbgInfo.base_addr = pool.base_addr;
    expectedDbgInfo.usable_size = claimSize + 1;
    expectedDbgInfo.room_size = BUFFER_1K_SIZE;
    expectedDbgInfo.room_occupied = true;
    expectedDbgInfo.is_last = true;
    expectedDbgInfo.is_first = true;

    testDbgData(&pool, &expectedDbgInfo, 1);
}

TEST(Mempool, mempool_claim_memory__NoFreeMemory__ErrorReturned)
{
    /* Phase 0: Claim all memory */
    const auto claimSize = BUFFER_1K_SIZE - mempool_calc_hdr_size();
    auto pool = initMempoolWith1KBuffer();
    claimMemory(&pool, claimSize);

    /* Phase 1: Claim memory second time */
    void* buff = nullptr;
    CHECK_EQUAL(mempool_status_out_of_memory, mempool_claim_memory(&pool, 10, &buff));
    POINTERS_EQUAL(nullptr, buff);
    CHECK_EQUAL(1, mempool_partitions_used(&pool));
}

TEST(Mempool, mempool_claim_memory__AfterInit__TwoMemoryRequests__SecondPartitionCreated)
{
    const auto claimSize = (BUFFER_1K_SIZE / 2) - mempool_calc_hdr_size();
    auto pool = initMempoolWith1KBuffer();
    void* buff = claimMemory(&pool, claimSize);

    mempool_debug_info expectedDbgInfo[2];

    /* Partition 0 */
    expectedDbgInfo[0].usable_space_addr = buff;
    expectedDbgInfo[0].base_addr = pool.base_addr;
    expectedDbgInfo[0].usable_size = claimSize;
    expectedDbgInfo[0].room_size = BUFFER_1K_SIZE / 2;
    expectedDbgInfo[0].room_occupied = true;
    expectedDbgInfo[0].is_last = false;
    expectedDbgInfo[0].is_first = true;

    /* Partition 1 */
    expectedDbgInfo[1].usable_space_addr = pool.base_addr + (BUFFER_1K_SIZE / 2) + mempool_calc_hdr_size();
    expectedDbgInfo[1].base_addr = pool.base_addr + (BUFFER_1K_SIZE / 2);
    expectedDbgInfo[1].usable_size = claimSize;
    expectedDbgInfo[1].room_size = BUFFER_1K_SIZE / 2;
    expectedDbgInfo[1].room_occupied = false;
    expectedDbgInfo[1].is_last = true;
    expectedDbgInfo[1].is_first = false;

    testDbgData(&pool, &expectedDbgInfo[0], 2);

    /* Claim memory second time */
    void* buff2 = claimMemory(&pool, claimSize);
    expectedDbgInfo[1].usable_space_addr = buff2;
    expectedDbgInfo[1].room_occupied = true;
    testDbgData(&pool, &expectedDbgInfo[0], 2);
}

TEST(Mempool, mempool_claim_memory__1KBuffer__CreateEightEqualPartitions__Success)
{
    const auto numPartitions = 8;
    const auto claimSize = (BUFFER_1K_SIZE / numPartitions) - mempool_calc_hdr_size();
    auto pool = initMempoolWith1KBuffer();
    void* buff = nullptr;

    for (size i = 0; i < numPartitions; ++i) {
        buff = nullptr;
        CHECK_EQUAL(mempool_status_ok, mempool_claim_memory(&pool, claimSize, &buff));
        CHECK(nullptr != buff);
    }
    CHECK_EQUAL(numPartitions, mempool_partitions_used(&pool));

    /* There should be no memory at this stage */
    CHECK_EQUAL(mempool_status_out_of_memory, mempool_claim_memory(&pool, 1, &buff));
}

TEST(Mempool, mempool_claim_memory__1KBuffer__Create8DifferentPartitions__Success)
{
    const auto numPartitions = 8;
    /* Header size should be subtracted */
    const size totalSizes[numPartitions] = {512, 128, 64, 64, 64, 64, 64, 64};
    auto pool = initMempoolWith1KBuffer();
    void* buff = nullptr;

    for (const auto& i : totalSizes) {
        auto claimSize = i - mempool_calc_hdr_size();
        buff = nullptr;
        CHECK_EQUAL(mempool_status_ok, mempool_claim_memory(&pool, claimSize, &buff));
        CHECK(nullptr != buff);
    }
    CHECK_EQUAL(numPartitions, mempool_partitions_used(&pool));

    /* There should be no memory at this stage */
    CHECK_EQUAL(mempool_status_out_of_memory, mempool_claim_memory(&pool, 1, &buff));
}

TEST(Mempool, mempool_free_memory__NullCases)
{
    auto pool = initMempoolWith1KBuffer();
    CHECK_EQUAL(mempool_status_nullptr, mempool_free_memory(nullptr, reinterpret_cast<void*>(1)));
    CHECK_EQUAL(mempool_status_nullptr, mempool_free_memory(&pool, nullptr));
}

TEST(Mempool, mempool_free_memory__FreeNonReservedPartition__Error)
{
    auto pool = initMempoolWith1KBuffer();
    void* mem = pool.base_addr + mempool_calc_hdr_size();
    CHECK_EQUAL(mempool_status_inv_memory, mempool_free_memory(&pool, mem));
    CHECK_EQUAL(1, mempool_partitions_used(&pool));
}

TEST(Mempool, mempool_free_memory__AllocateAndFree__Success)
{
    auto pool = initMempoolWith1KBuffer();
    auto dst = claimMemory(&pool, BUFFER_1K_SIZE - mempool_calc_hdr_size());
    CHECK_EQUAL(1, mempool_partitions_used(&pool));

    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, dst));
    CHECK_EQUAL(1, mempool_partitions_used(&pool));
}

TEST(Mempool, mempool_free_memory__AllocateMemoryTwice__FreeMemoryTwice__Success)
{
    auto pool = initMempoolWith1KBuffer();
    void* ptr1 = claimMemory(&pool, 10);
    void* ptr2 = claimMemory(&pool, 20);
    CHECK(mempool_partitions_used(&pool) > 1);
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, ptr1));
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, ptr2));
    CHECK_EQUAL(1, mempool_partitions_used(&pool));
}

TEST(Mempool, mempool_free_memory__TwoPartitions__LeftOccupied__MergedAfterLeftDeleted)
{
    const size claimSize = (BUFFER_1K_SIZE / 2) - mempool_calc_hdr_size();
    auto pool = initMempoolWith1KBuffer();
    auto dst = claimMemory(&pool, claimSize);
    CHECK_EQUAL(2, mempool_partitions_used(&pool));

    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, dst));
    CHECK_EQUAL(1, mempool_partitions_used(&pool));
}

TEST(Mempool, mempool_free_memory__TwoPartitions__RightOccupied__MergedAfterRightDeleted)
{
    const size claimSize = (BUFFER_1K_SIZE / 2) - mempool_calc_hdr_size();
    auto pool = initMempoolWith1KBuffer();
    auto dst = claimMemory(&pool, claimSize); /* Left partition */
    auto dst2 = claimMemory(&pool, claimSize); /* Right partition */
    CHECK_EQUAL(2, mempool_partitions_used(&pool));

    /* Delete left partition */
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, dst));
    CHECK_EQUAL(2, mempool_partitions_used(&pool));

    /* Delete right partition - after this only single one should exist */
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, dst2));
    CHECK_EQUAL(1, mempool_partitions_used(&pool));
}

TEST(Mempool, mempool_free_memory__TwoPartitions__BothOccupied__NotMergedAfterLeftIsDeleted)
{
    const size claimSize = (BUFFER_1K_SIZE / 2) - mempool_calc_hdr_size();
    auto pool = initMempoolWith1KBuffer();

    /* Create two partitions */
    auto dst = claimMemory(&pool, claimSize);
    claimMemory(&pool, claimSize);
    CHECK_EQUAL(2, mempool_partitions_used(&pool));

    /* Delete left partition */
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, dst));
    CHECK_EQUAL(2, mempool_partitions_used(&pool));
}

TEST(Mempool, mempool_free_memory__TwoPartitions__BothOccupied__NotMergedAfterRightIsDeleted)
{
    const size claimSize = (BUFFER_1K_SIZE / 2) - mempool_calc_hdr_size();
    auto pool = initMempoolWith1KBuffer();

    /* Create two partitions */
    claimMemory(&pool, claimSize);
    auto dst = claimMemory(&pool, claimSize);
    CHECK_EQUAL(2, mempool_partitions_used(&pool));

    /* Delete right partition */
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, dst));
    CHECK_EQUAL(2, mempool_partitions_used(&pool));
}

TEST(Mempool, mempool_free_memory__TwoPartitions__BothOccupied__MergedAfterLeftAndRightAreDeleted)
{
    const size claimSize = (BUFFER_1K_SIZE / 2) - mempool_calc_hdr_size();
    auto pool = initMempoolWith1KBuffer();

    /* Create two partitions */
    auto dst = claimMemory(&pool, claimSize);
    auto dst2 = claimMemory(&pool, claimSize);
    CHECK_EQUAL(2, mempool_partitions_used(&pool));

    /* Delete both partitions */
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, dst2));
    CHECK_EQUAL(2, mempool_partitions_used(&pool));
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, dst));
    CHECK_EQUAL(1, mempool_partitions_used(&pool));
}

TEST(Mempool, mempool_free_memory__AllocateAndFreeBuffers__Success)
{
    auto pool = initMempoolWith1KBuffer();

    /* Allocate some memory */
    auto dst1 = claimMemory(&pool, 10);
    auto dst2 = claimMemory(&pool, 64);
    auto dst3 = claimMemory(&pool, 1);
    auto dst4 = claimMemory(&pool, 128);
    auto dst5 = claimMemory(&pool, 32);
    CHECK_EQUAL(7, mempool_partitions_used(&pool));

    /* Free memory from the middle */
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, dst3));
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, dst2));

    /* Free remaining buffers */
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, dst1));
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, dst4));
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, dst5));
    CHECK_EQUAL(1, mempool_partitions_used(&pool));
}

TEST(Mempool, mempool_memory_used__NullPassed__ZeroReturned)
{
    CHECK_EQUAL(0, mempool_memory_used(nullptr));
}

TEST(Mempool, mempool_memory_used__AfterInit__HdrMemIsUsed)
{
    auto pool = initMempoolWith1KBuffer();
    size memUsed = mempool_memory_used(&pool);
    CHECK_EQUAL(mempool_calc_hdr_size(), memUsed);

    /* Try allocating free memory */
    claimMemory(&pool, BUFFER_1K_SIZE - memUsed);
}

TEST(Mempool, mempool_memory_used__ClaimAllMemory__TotalSizeReturned)
{
    auto pool = initMempoolWith1KBuffer();
    claimMemory(&pool, BUFFER_1K_SIZE - mempool_calc_hdr_size());
    CHECK_EQUAL(BUFFER_1K_SIZE, mempool_memory_used(&pool));
}

TEST(Mempool, mempool_memory_used__MultipleClaims__ReturnedValueVaries)
{
    auto pool = initMempoolWith1KBuffer();
    void* ptr1 = claimMemory(&pool, 512 - mempool_calc_hdr_size());
    CHECK_EQUAL(512 + mempool_calc_hdr_size(), mempool_memory_used(&pool));
    void* ptr2 = claimMemory(&pool, 256 - mempool_calc_hdr_size());
    CHECK_EQUAL(512 + 256 + mempool_calc_hdr_size(), mempool_memory_used(&pool));

    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, ptr1));
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, ptr2));
    CHECK_EQUAL(mempool_calc_hdr_size(), mempool_memory_used(&pool));
}