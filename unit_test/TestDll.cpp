#include "TestRunner.h"
#include "dll.h"

/* User data type for testing dll API */
struct UserData
{
    u32 magicNumber;
};

TEST_GROUP(Dll)
{
    static const u32 MAGIC_NUMBER = 0xAABBCCDD;
    UserData userData {};

    void setup() override
    {
        userData.magicNumber = MAGIC_NUMBER;
    }

    void teardown() override
    {
        /* Nothing to do here */
    }

    void* getUserDataPtr()
    {
        return static_cast<void*>(&userData);
    }
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(Dll, NullCases)
{
    CHECK_EQUAL(dll_status_iptr, dll_create(nullptr, getUserDataPtr(), nullptr));
}

TEST(Dll, dll_create__DllInitialized)
{
    dll_node_decay_fn decayFn = nullptr;
    dll_node dll;
    CHECK_EQUAL(dll_status_ok, dll_create(&dll, getUserDataPtr(), decayFn));
    CHECK_EQUAL(nullptr, dll_get_prev_node(&dll));
    CHECK_EQUAL(nullptr, dll_get_next_node(&dll));
    MEMCMP_EQUAL(getUserDataPtr(), dll_get_user_data(&dll), sizeof(UserData));
    CHECK_EQUAL(decayFn, dll_get_decay_fn(&dll));
}


