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
