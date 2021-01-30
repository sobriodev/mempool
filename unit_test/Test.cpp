#include "TestRunner.h"
#include "dummy.h"

TEST_GROUP(Test)
{
    /* Nothing to do here at the moment */
};

TEST(Test, get100__Returns100)
{
    CHECK_EQUAL(100, get100());
}

