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

TEST(Bit, BIT_32_SET_MUL__Misc__CorrectResults)
{
    /* Case 0: set lower nibble to 0x0F */
    u8 word = 0b11110000;
    const u8 msk = 0x0F;
    const u8 pos = 0;
    BIT_32_SET_MUL(word, msk, pos, 0x0F);
    CHECK_EQUAL(0xFF, word);

    /* Case 1: set upper nibble to new value */
    u8 word1 = 0b00110101;
    const u8 msk1 = 0x0F;
    const u8 pos1 = 4;
    BIT_32_SET_MUL(word1, msk1, pos1, 0b1100);
    CHECK_EQUAL(0b11000101, word1);

    /* Case 2: clear entire word */
    u8 word2 = 0b11001100;
    const u8 msk2 = 0xFF;
    const u8 pos2 = 0;
    BIT_32_SET_MUL(word2, msk2, pos2, 0);
    CHECK_EQUAL(0, word2);

    /* Case 3: set single bit only with u32 word */
    u32 word3 = 0b11101111;
    const u8 msk3 = 1;
    const u8 pos3 = 4;
    BIT_32_SET_MUL(word3, msk3, pos3, 1);
    CHECK_EQUAL(0xFF, word3);
}

TEST(Bit, BIT_32_GET_MUL__Misc__ValidResults)
{
    /* Case 0: single bit */
    const u8 word = 0b00001111;
    const u8 msk = 1;
    const u8 pos = 3;
    CHECK_EQUAL(1, BIT_32_GET_MUL(word, msk, pos));

    /* Case 2: mask */
    const u16 word2 = 0xFFAC;
    const u16 msk2 = 0x0F;
    const u16 pos2 = 4;
    CHECK_EQUAL(0x0A, BIT_32_GET_MUL(word2, msk2, pos2));

    /* Case 3: another mask */
    const u32 word3 = 0xABCD0000;
    const u8 msk3 = 0b00000111;
    const u8 pos3 = 28;
    CHECK_EQUAL(0b00000010, BIT_32_GET_MUL(word3, msk3, pos3));
}