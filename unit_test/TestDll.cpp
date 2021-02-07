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
    auto createDllNode()
    {
        dll_node head;
        auto status = dll_create(&head, getUserDataPtr());
        CHECK_EQUAL(dll_status_ok, status);
        return head;
    }

    auto createDllOnHeap(size num_of_extra_nodes)
    {
        auto head = new dll_node;
        CHECK_EQUAL(dll_status_ok, dll_create(head, getUserDataPtr()));
        dll_status status;
        for (size i = 0; i < num_of_extra_nodes; ++i) {
            auto new_node = new dll_node;
            CHECK_EQUAL(dll_status_ok, dll_create(new_node, getUserDataPtr()));
            head = dll_node_insert_begin(head, new_node, &status);
        }
        return head;
    }
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */
/* ------------------------------------------------------------ */

TEST(Dll, dll_create__NullCases)
{
    CHECK_EQUAL(dll_status_iptr, dll_create(nullptr, getUserDataPtr()));
}

TEST(Dll, dll_destroy__NullCases)
{
    CHECK_EQUAL(dll_status_iptr, dll_destroy(nullptr, [](dll_node* node){}));
    auto head = createDllNode();
    CHECK_EQUAL(dll_status_iptr, dll_destroy(&head, nullptr));
}

TEST(Dll, dll_node_insert_begin__NullCases)
{
    auto head = createDllNode();
    dll_status status;
    dll_node_insert_begin(&head, nullptr, &status);
    CHECK_EQUAL(dll_status_iptr, status);
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
    auto head = createDllNode();
    CHECK_EQUAL(dll_status_ok, dll_destroy(&head, [](dll_node* node){
        auto userData = static_cast<UserData*>(node->user_data);
        userData->magicNumber = newMagicNumber;
    }));
    CHECK_EQUAL(newMagicNumber, static_cast<UserData*>(dll_get_user_data(&head))->magicNumber);
}

TEST(Dll, dll_destroy__OnlyHeadOnHeap__MemoryFreed)
{
    const size numOfExtraNodes = 0;
    auto dll = createDllOnHeap(numOfExtraNodes);
    dll_destroy(dll, [](dll_node* node){
        delete node;
    });
    /* Cpputest would throw an error if memory leakages occurred */
}

TEST(Dll, dll_destroy__TwentyNodesOnHeap__MemoryFreed)
{
    const size numOfExtraNodes = 19;
    auto dll = createDllOnHeap(numOfExtraNodes);
    dll_destroy(dll, [](dll_node* node){
        delete node;
    });
    /* Cpputest would throw an error if memory leakages occurred */
}

TEST(Dll, dll_node_insert_begin__WithoutCollectingTheStatus__Success)
{
    auto node = createDllNode();
    auto newHead = dll_node_insert_begin(nullptr, &node, nullptr);
    POINTERS_EQUAL(&node, newHead);
}

TEST(Dll, dll_node_insert_begin__WithoutHead__NextNodeIsNull)
{
    dll_node* head = nullptr;
    auto newNode = createDllNode();
    dll_status status;
    auto newHead = dll_node_insert_begin(head, &newNode, &status);
    POINTERS_EQUAL(&newNode, newHead);
    POINTERS_EQUAL(nullptr, dll_get_next_node(newHead));
}

TEST(Dll, dll_node_insert_begin__WithHead__HeadNodeIsSuccessorAfterOperation)
{
    auto head = createDllNode();
    auto newNode = createDllNode();
    dll_status status;
    auto newHead = dll_node_insert_begin(&head, &newNode, &status);
    CHECK_EQUAL(dll_status_ok, status);
    POINTERS_EQUAL(&newNode, newHead);
    POINTERS_EQUAL(&head, dll_get_next_node(newHead));
    POINTERS_EQUAL(newHead, dll_get_prev_node(&head));
}

TEST(Dll, dll_node_insert_begin__MultipleTimes__ValidDllCreated)
{
    dll_node nodes[] = {createDllNode(), createDllNode(), createDllNode(), createDllNode()};
    auto head = createDllNode();

    dll_status status;
    dll_node* new_head = &head;

    /* Insert nodes */
    for (auto& node : nodes)
    {
        new_head = dll_node_insert_begin(new_head, &node, &status);
        CHECK_EQUAL(dll_status_ok, status);
    }
    /* Check predecessors and successors */
    POINTERS_EQUAL(nullptr, nodes[3].prev);
    POINTERS_EQUAL(new_head, &nodes[3]);

    POINTERS_EQUAL(&nodes[3], nodes[2].prev);
    POINTERS_EQUAL(new_head->next, &nodes[2]);

    POINTERS_EQUAL(&nodes[2], nodes[1].prev);
    POINTERS_EQUAL(new_head->next->next, &nodes[1]);

    POINTERS_EQUAL(&nodes[1], nodes[0].prev);
    POINTERS_EQUAL(new_head->next->next->next, &nodes[0]);

    POINTERS_EQUAL(&nodes[0], head.prev);
    POINTERS_EQUAL(new_head->next->next->next->next, &head);
}