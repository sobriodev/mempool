#include "TestRunner.h"
#include "mempool.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

TEST_GROUP(MempoolSanityCheck)
{
    static const size BUFFER_2K_SIZE = 2048;
    char* buffer2K = nullptr;

    void setup() override
    {
        buffer2K = new char[BUFFER_2K_SIZE];
    }

    void teardown() override
    {
        delete[] buffer2K;
    }

    void initMempool(mempool_instance* pool) const
    {
        pool->base_addr = &buffer2K[0];
        pool->size = BUFFER_2K_SIZE;
        CHECK_EQUAL(mempool_status_ok, mempool_init(pool));
    }
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(MempoolSanityCheck, mempool_free_memory__AfterInit__InvalidPointerGiven__Error)
{
    mempool_instance pool;
    initMempool(&pool);

    /* Free memory that was not allocated */
    void* ptr = pool.base_addr + mempool_calc_hdr_size();
    CHECK_EQUAL(mempool_status_inv_memory,mempool_free_memory(&pool, ptr));
    CHECK_EQUAL(1, mempool_partitions_used(&pool));
}

TEST(MempoolSanityCheck, mempool_free_memory__AfterInit__ReservedAndFree_Ok)
{
    const size halfMemWithoutHdr = (BUFFER_2K_SIZE / 2) - mempool_calc_hdr_size();
    mempool_instance pool;
    initMempool(&pool);

    void* ptr = nullptr;
    void* ptr2 = nullptr;
    CHECK_EQUAL(mempool_status_ok, mempool_claim_memory(&pool, halfMemWithoutHdr, &ptr));
    CHECK_EQUAL(mempool_status_ok, mempool_claim_memory(&pool, halfMemWithoutHdr, &ptr2));
    CHECK(mempool_partitions_used(&pool) > 1);
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, ptr));
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, ptr2));
    CHECK_EQUAL(1, mempool_partitions_used(&pool));
}