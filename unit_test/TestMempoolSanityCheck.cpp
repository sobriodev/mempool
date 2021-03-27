#include "TestRunner.h"
#include "mempool.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

TEST_GROUP(MempoolSanityCheck)
{
    void setup() override {}
    void teardown() override {}
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(MempoolSanityCheck, mempool_free_memory__AfterInit__InvalidPointerGiven__Error)
{
    const size poolLen = 2048;
    char* poolBuffer = new char[poolLen];

    mempool_instance pool;
    pool.base_addr = poolBuffer;
    pool.size = poolLen;
    CHECK_EQUAL(mempool_status_ok, mempool_init(&pool));

    /* Free memory that was not allocated */
    void* ptr = pool.base_addr + mempool_calc_hdr_size();
    CHECK_EQUAL(mempool_status_inv_memory,mempool_free_memory(&pool, ptr));
    CHECK_EQUAL(1, mempool_partitions_used(&pool));

    delete[] poolBuffer;
}

TEST(MempoolSanityCheck, mempool_free_memory__AfterInit__ReservedAndFree_Ok)
{
    const size poolLen = 2048;
    const size halfMemWithoutHdr = (poolLen / 2) - mempool_calc_hdr_size();
    char* poolBuffer = new char[poolLen];

    mempool_instance pool;
    pool.base_addr = poolBuffer;
    pool.size = poolLen;
    CHECK_EQUAL(mempool_status_ok, mempool_init(&pool));

    void* ptr = nullptr;
    void* ptr2 = nullptr;
    CHECK_EQUAL(mempool_status_ok, mempool_claim_memory(&pool, halfMemWithoutHdr, &ptr));
    CHECK_EQUAL(mempool_status_ok, mempool_claim_memory(&pool, halfMemWithoutHdr, &ptr2));
    CHECK(mempool_partitions_used(&pool) > 1);
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, ptr));
    CHECK_EQUAL(mempool_status_ok, mempool_free_memory(&pool, ptr2));
    CHECK_EQUAL(1, mempool_partitions_used(&pool));
    delete[] poolBuffer;
}