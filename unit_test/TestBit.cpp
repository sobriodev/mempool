#include "TestRunner.h"
#include "bit.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

TEST_GROUP(Bit)
{
    void setup() override {}
    void teardown() override {}
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(Bit, BIT_32_GET_AT_POS__MiscValues__CorrectResults)
{
    CHECK_EQUAL(1, BIT_32_GET_AT_POS(0));
    CHECK_EQUAL(64, BIT_32_GET_AT_POS(6));
    CHECK_EQUAL(128, BIT_32_GET_AT_POS(7));
    CHECK_EQUAL(131072, BIT_32_GET_AT_POS(17));
    CHECK_EQUAL(2147483648, BIT_32_GET_AT_POS(31));
}

TEST(Bit, BIT_32_NOT__MiscValues__ValidResults)
{
    CHECK_EQUAL(4294967295, BIT_32_NOT(0));

    u8 val = 255;
    CHECK_EQUAL(0, (u8)BIT_32_NOT(val));

    i16 anotherVal = 0;
    CHECK_EQUAL(-1, (i16)BIT_32_NOT(anotherVal));
}

TEST(Bit, BIT_32_SET__MiscValus__ValidResults)
{
    u8 v = 0;
    BIT_32_SET(v, 0);
    CHECK_EQUAL(1, v);
    BIT_32_SET(v, 1);
    CHECK_EQUAL(3, v);
    /* 8th bit is out of range here - nothing should change */
    BIT_32_SET(v, 8);
    CHECK_EQUAL(3, v);

    i32 v2 = 0;
    BIT_32_SET(v2, 31);
    BIT_32_SET(v2, 0);
    CHECK_EQUAL(INT32_MIN + 1, v2);
}

TEST(Bit, BIT_32_CLR__MiscValues__ValidResults)
{
    u16 v = INT16_MAX;
    BIT_32_CLR(v, 15);
    CHECK_EQUAL(UINT16_MAX - 32768, v);

    i32 v2 = 123;
    BIT_32_CLR(v2, 0);
    BIT_32_CLR(v2, 1);
    CHECK_EQUAL(120, v2);
}

TEST(Bit, BIT_32_IS_SET__MiscValues__ValidResults)
{
    u8 someValue = 0b11101010;
    CHECK_TRUE(BIT_32_IS_SET(someValue, 1));
    CHECK_FALSE(BIT_32_IS_SET(someValue, 0));
    CHECK_TRUE(BIT_32_IS_SET(someValue, 7));

    i32 anotherValue = -1;
    CHECK_TRUE(BIT_32_IS_SET(anotherValue, 31));
}

TEST(Bit, BIT_32_IS_NOT_SET__MiscValues__ValidResults)
{
    u8 someValue = 0b10101010;
    CHECK_TRUE(BIT_32_IS_NOT_SET(someValue, 0));
    CHECK_FALSE(BIT_32_IS_NOT_SET(someValue, 1));
    CHECK_TRUE(BIT_32_IS_NOT_SET(someValue, 6));

    CHECK_FALSE(BIT_32_IS_SET(63, 8));
}