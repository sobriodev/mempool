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

    /* Node will be eventually copied, thus there is no need to destroy the list */
    auto createDllHead()
    {
        dll_node head;
        auto status = dll_create(&head, getUserDataPtr());
        CHECK_EQUAL(dll_status_ok, status);
        return head;
    }
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(Dll, NullCases)
{
    CHECK_EQUAL(dll_status_iptr, dll_create(nullptr, getUserDataPtr()));

    CHECK_EQUAL(dll_status_iptr, dll_destroy(nullptr, [](dll_node* node){}));
    auto head = createDllHead();
    CHECK_EQUAL(dll_status_iptr, dll_destroy(&head, nullptr));
}

TEST(Dll, dll_create__DllInitialized)
{
    dll_node dll;
    CHECK_EQUAL(dll_status_ok, dll_create(&dll, getUserDataPtr()));
    CHECK_EQUAL(nullptr, dll_get_prev_node(&dll));
    CHECK_EQUAL(nullptr, dll_get_next_node(&dll));
    MEMCMP_EQUAL(getUserDataPtr(), dll_get_user_data(&dll), sizeof(UserData));
}

TEST(Dll, dll_destroy__HeadNodeOnly__DecayFunctionCalled)
{
    const u32 newMagicNumber = 0xFAFAFAFA;
    auto head = createDllHead();
    CHECK_EQUAL(dll_status_ok, dll_destroy(&head, [](dll_node* node){
        auto userData = static_cast<UserData*>(node->user_data);
        userData->magicNumber = newMagicNumber;
    }));
    CHECK_EQUAL(newMagicNumber, static_cast<UserData*>(dll_get_user_data(&head))->magicNumber);
}

/* TODO implement more tests testing dll_destroy() when API for adding new nodes will be available */