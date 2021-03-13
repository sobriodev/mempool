#include "TestRunner.h"
#include "dll.h"

/* ------------------------------------------------------------ */
/* --------------------- Private data types ------------------- */
/* ------------------------------------------------------------ */

/* User data type for testing dll API */
struct UserData
{
    u32 magicNumber;
};

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

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
        auto status = dll_node_create(&head, getUserDataPtr());
        CHECK_EQUAL(dll_status_ok, status);
        return head;
    }

    auto createDllOnHeap(size num_of_extra_nodes)
    {
        auto head = new dll_node;
        CHECK_EQUAL(dll_status_ok, dll_node_create(head, getUserDataPtr()));
        auto status = dll_status_nok;
        for (size i = 0; i < num_of_extra_nodes; ++i) {
            auto newNode = new dll_node;
            CHECK_EQUAL(dll_status_ok, dll_node_create(newNode, getUserDataPtr()));
            head = dll_node_insert_begin(head, newNode, &status);
        }
        return head;
    }

    static auto getSimpleNodeDecayFn()
    {
        return [](dll_node* node) {
            delete node;
        };
    }

    static auto getSimpleNodeCmpFn()
    {
        return [](const void* user_data) {
            auto userDataPtr = static_cast<const UserData*>(user_data);
            return (userDataPtr->magicNumber == MAGIC_NUMBER);
        };
    }

    static void destroyDllOnHeap(dll_node* dll)
    {
        CHECK_EQUAL(dll_status_ok, dll_destroy(dll, getSimpleNodeDecayFn()));
    }
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(Dll, dll_node_create__NullCases)
{
    CHECK_EQUAL(dll_status_iptr, dll_node_create(nullptr, getUserDataPtr()));
}

TEST(Dll, dll_destroy__NullCases)
{
    CHECK_EQUAL(dll_status_iptr, dll_destroy(nullptr, [](dll_node* node) {}));
    auto head = createDllNode();
    CHECK_EQUAL(dll_status_iptr, dll_destroy(&head, nullptr));
}

TEST(Dll, dll_node_insert_begin__NullCases)
{
    auto head = createDllNode();
    auto status = dll_status_ok;
    dll_node_insert_begin(&head, nullptr, &status);
    CHECK_EQUAL(dll_status_iptr, status);
}

TEST(Dll, dll_node_insert_end__NullCases)
{
    auto head = createDllNode();
    auto status = dll_status_ok;
    dll_node_insert_end(&head, nullptr, &status);
    CHECK_EQUAL(dll_status_iptr, status);
}

TEST(Dll, dll_node_insert_before__NullCases)
{
    auto node1 = createDllNode();
    auto node2 = createDllNode();
    auto s = dll_status_ok;
    dll_node_insert_before(nullptr, &node2, &s);
    CHECK_EQUAL(dll_status_iptr, s);

    s = dll_status_ok;
    dll_node_insert_before(&node1, nullptr, &s);
    CHECK_EQUAL(dll_status_iptr, s);
}

TEST(Dll, dll_node_insert_after__NullCases)
{
    auto node1 = createDllNode();
    auto node2 = createDllNode();
    auto s = dll_status_ok;
    dll_node_insert_after(nullptr, &node2, &s);
    CHECK_EQUAL(dll_status_iptr, s);

    s = dll_status_ok;
    dll_node_insert_after(&node1, nullptr, &s);
    CHECK_EQUAL(dll_status_iptr, s);
}

TEST(Dll, dll_node_delete_before__NullCases)
{
    auto s = dll_status_ok;
    dll_node_delete_before(nullptr, &s, nullptr);
    CHECK_EQUAL(dll_status_iptr, s);
}

TEST(Dll, dll_node_delete_after__NullCases)
{
    auto s = dll_status_ok;
    dll_node_delete_after(nullptr, &s, nullptr);
    CHECK_EQUAL(dll_status_iptr, s);
}

TEST(Dll, dll_node_delete_begin__NullCases)
{
    auto s = dll_status_ok;
    dll_node_delete_begin(nullptr, &s, nullptr);
    CHECK_EQUAL(dll_status_iptr, s);
}

TEST(Dll, dll_node_delete_end__NullCases)
{
    auto s = dll_status_ok;
    dll_node_delete_end(nullptr, &s, nullptr);
    CHECK_EQUAL(dll_status_iptr, s);
}

TEST(Dll, dll_node_create__DllInitialized)
{
    dll_node dll;
    CHECK_EQUAL(dll_status_ok, dll_node_create(&dll, getUserDataPtr()));
    CHECK_EQUAL(nullptr, dll_get_prev_node(&dll));
    CHECK_EQUAL(nullptr, dll_get_next_node(&dll));
    MEMCMP_EQUAL(getUserDataPtr(), dll_get_user_data(&dll), sizeof(UserData));
}

TEST(Dll, dll_destroy__HeadNodeOnly__DecayFunctionCalled)
{
    const u32 newMagicNumber = 0xFAFAFAFA;
    auto head = createDllNode();
    CHECK_EQUAL(dll_status_ok, dll_destroy(&head, [](dll_node* node) {
        auto userData = static_cast<UserData*>(node->user_data);
        userData->magicNumber = newMagicNumber;
    }));
    CHECK_EQUAL(newMagicNumber, static_cast<UserData*>(dll_get_user_data(&head))->magicNumber);
}

TEST(Dll, dll_destroy__OnlyHeadOnHeap__MemoryFreed)
{
    const size numOfExtraNodes = 0;
    auto dll = createDllOnHeap(numOfExtraNodes);
    destroyDllOnHeap(dll);
    /* Cpputest would throw an error if memory leakages occurred */
}

TEST(Dll, dll_destroy__TwentyNodesOnHeap__MemoryFreed)
{
    const size numOfExtraNodes = 19;
    auto dll = createDllOnHeap(numOfExtraNodes);
    destroyDllOnHeap(dll);
    /* Cpputest would throw an error if memory leakages occurred */
}

TEST(Dll, dll_node_insert_begin__WithoutCollectingTheStatus__Success)
{
    auto node = createDllNode();
    dll_node_insert_begin(nullptr, &node, nullptr);
    /* Failure is not gathered but nothing nasty should happen */
}

TEST(Dll, dll_node_insert_begin__WithHead__HeadNodeIsSuccessorAfterOperation)
{
    auto head = createDllNode();
    auto newNode = createDllNode();
    auto status = dll_status_nok;
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

    auto status = dll_status_nok;
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

TEST(Dll, dll_find_tail__NullPassed__NullReturned)
{
    POINTERS_EQUAL(nullptr, dll_find_tail(nullptr));
}

TEST(Dll, dll_find_tail__OneNode__ValidNodeReturned)
{
    auto head = createDllNode();
    auto last = dll_find_tail(&head);
    POINTERS_EQUAL(&head, last);
}

TEST(Dll, dll_find_tail__ThreeNodes__ValidNodeReturned)
{
    auto list = createDllOnHeap(2);
    POINTERS_EQUAL(list->next->next, dll_find_tail(list));
    /* Destroy the list explicitly since its nodes are allocated on the heap */
    destroyDllOnHeap(list);
}

TEST(Dll, dll_find_head__NullPassed__NullReturned)
{
    POINTERS_EQUAL(nullptr, dll_find_head(nullptr));
}

TEST(Dll, dll_find_head__OneNode__TheSameNodeReturned)
{
    auto node = createDllNode();
    POINTERS_EQUAL(&node, dll_find_head(&node));
}

TEST(Dll, dll_find_head__FiveNodes__StartingFromTheMiddle)
{
    auto list = createDllOnHeap(4);
    auto startNode = list->next->next;
    POINTERS_EQUAL(list, dll_find_head(startNode));
    destroyDllOnHeap(list);
}

TEST(Dll, dll_find_head__TwoNodes__StartingFromTheEnd)
{
    auto list = createDllOnHeap(1);
    auto last = list->next;
    POINTERS_EQUAL(list, dll_find_head(last));
    destroyDllOnHeap(list);
}

TEST(Dll, dll_node_insert_end__OneNode__Success)
{
    auto list = createDllNode();
    auto newNode = createDllNode();
    auto status = dll_status_nok;
    auto nh = dll_node_insert_end(&list, &newNode, &status);
    CHECK_EQUAL(dll_status_ok, status);
    POINTERS_EQUAL(&list, nh);
    POINTERS_EQUAL(dll_get_next_node(&list), &newNode);
    POINTERS_EQUAL(&list, dll_get_prev_node(&newNode));
}

TEST(Dll, dll_node_insert_before__OneNode__Success)
{
    auto dll = createDllOnHeap(0);
    auto newNode = createDllOnHeap(0);
    auto status = dll_status_nok;
    auto nh = dll_node_insert_before(dll, newNode, &status);
    CHECK_EQUAL(dll_status_ok, status);
    POINTERS_EQUAL(newNode, nh);
    POINTERS_EQUAL(dll, dll_get_next_node(nh));
    POINTERS_EQUAL(nh, dll_get_prev_node(dll));
    destroyDllOnHeap(nh);
}

TEST(Dll, dll_node_insert_before__BetweenTwoNodes__Success)
{
    auto list = createDllOnHeap(1);
    auto first = list;
    auto second = dll_get_next_node(first);
    auto newNode = createDllOnHeap(0);
    auto stat = dll_status_nok;
    auto newHead = dll_node_insert_before(second, newNode, &stat);
    CHECK_EQUAL(dll_status_ok, stat);
    POINTERS_EQUAL(first, newHead);
    POINTERS_EQUAL(newNode, first->next);
    POINTERS_EQUAL(first, newNode->prev);
    POINTERS_EQUAL(second, newNode->next);
    POINTERS_EQUAL(newNode, second->prev);
    destroyDllOnHeap(newHead);
}

TEST(Dll, dll_node_insert_after__OneNode__Success)
{
    auto list = createDllOnHeap(0);
    auto newNode = createDllOnHeap(0);
    auto status = dll_status_nok;
    auto newHead = dll_node_insert_after(list, newNode, &status);
    CHECK_EQUAL(dll_status_ok, status);
    POINTERS_EQUAL(list, newHead);
    POINTERS_EQUAL(newNode, newHead->next);
    POINTERS_EQUAL(newHead, newNode->prev);
    destroyDllOnHeap(newHead);
}

TEST(Dll, dll_node_insert_after__BetweenTwoNodes__Success)
{
    auto list = createDllOnHeap(1);
    auto first = list;
    auto second = dll_get_next_node(first);
    auto newNode = createDllOnHeap(0);
    auto stat = dll_status_nok;
    auto newHead = dll_node_insert_after(first, newNode, &stat);
    CHECK_EQUAL(dll_status_ok, stat);
    POINTERS_EQUAL(first, newHead);
    POINTERS_EQUAL(newNode, first->next);
    POINTERS_EQUAL(first, newNode->prev);
    POINTERS_EQUAL(second, newNode->next);
    POINTERS_EQUAL(newNode, second->prev);
    destroyDllOnHeap(newHead);
}

TEST(Dll, dll_node_delete_before__OneNode__NothingIsDone)
{
    auto list = createDllNode();
    auto status = dll_status_nok;
    auto newHead = dll_node_delete_before(&list, &status, nullptr);
    POINTERS_EQUAL(&list, newHead);
    CHECK_EQUAL(dll_status_ok, status);
}

TEST(Dll, dll_node_delete_before__TwoNodes__NodeRemoved)
{
    auto list = createDllOnHeap(1);
    auto tail = list->next;
    auto stat = dll_status_nok;
    /* Pass decay function also since the list is allocated on the heap */
    auto nh = dll_node_delete_before(tail, &stat, getSimpleNodeDecayFn());
    CHECK_EQUAL(dll_status_ok, stat);
    POINTERS_EQUAL(tail, nh);
    POINTERS_EQUAL(nullptr, nh->prev);
    destroyDllOnHeap(nh);
}

TEST(Dll, dll_node_delete_before__ThreeNodes__MiddleNodeRemoved__TwoNodesExist)
{
    auto list = createDllOnHeap(2);
    auto tail = list->next->next;
    auto status = dll_status_nok;
    /* Delete the middle node with a call to decay function */
    auto nh = dll_node_delete_before(tail, &status, getSimpleNodeDecayFn());
    CHECK_EQUAL(dll_status_ok, status);
    POINTERS_EQUAL(list, nh);
    POINTERS_EQUAL(tail, list->next);
    POINTERS_EQUAL(list, tail->prev);
    destroyDllOnHeap(nh);
}

TEST(Dll, dll_node_delete_after__OneNode__NothingIsDone)
{
    auto list = createDllOnHeap(0);
    auto status = dll_status_nok;
    /* Pass decay function just in case. The function should not be called */
    auto nh = dll_node_delete_after(list, &status, getSimpleNodeDecayFn());
    CHECK_EQUAL(dll_status_ok, status);
    POINTERS_EQUAL(list, nh);
    destroyDllOnHeap(nh);
}

TEST(Dll, dll_node_delete_after__TwoNodesOnHeap__SingleNodeAfterOperation)
{
    auto head = createDllOnHeap(1);
    auto s = dll_status_nok;
    auto newHead = dll_node_delete_after(head, &s, getSimpleNodeDecayFn());
    CHECK_EQUAL(dll_status_ok, s);
    POINTERS_EQUAL(head, newHead);
    POINTERS_EQUAL(nullptr, head->next);
    destroyDllOnHeap(newHead);
}

TEST(Dll, dll_node_delete_after__ThreeNodes__RemoveMiddleNode__TwoNodesExist)
{
    auto list = createDllOnHeap(2);
    auto tail = list->next->next;
    auto stat = dll_status_nok;
    auto nh = dll_node_delete_after(list, &stat, getSimpleNodeDecayFn());
    CHECK_EQUAL(dll_status_ok, stat);
    POINTERS_EQUAL(list, nh);
    POINTERS_EQUAL(tail, nh->next);
    POINTERS_EQUAL(list, tail->prev);
    destroyDllOnHeap(nh);
}

TEST(Dll, dll_node_delete_begin__OnlyHead__NullReturned)
{
    auto head = createDllOnHeap(0);
    auto stat = dll_status_nok;
    auto nh = dll_node_delete_begin(head, &stat, getSimpleNodeDecayFn());
    CHECK_EQUAL(dll_status_ok, stat);
    POINTERS_EQUAL(nullptr, nh);
    /* There should not be any existing nodes at this point. Nothing to free here */
}

TEST(Dll, dll_node_delete_begin__ManyNodes__FirstNodeDeleted)
{
    auto head = createDllOnHeap(10);
    auto next = head->next;
    auto stat = dll_status_nok;
    auto nh = dll_node_delete_begin(head, &stat, getSimpleNodeDecayFn());
    CHECK_EQUAL(dll_status_ok, stat);
    POINTERS_EQUAL(next, nh);
    destroyDllOnHeap(nh);
}

TEST(Dll, dll_node_delete_end__OnlyHead__NullReturned)
{
    auto dll = createDllOnHeap(0);
    auto s = dll_status_nok;
    auto nh = dll_node_delete_end(dll, &s, getSimpleNodeDecayFn());
    CHECK_EQUAL(dll_status_ok, s);
    POINTERS_EQUAL(nullptr, nh);
    /* There should not be any existing nodes at this point. Nothing to free here */
}

TEST(Dll, dll_node_delete_end__ThreeNodes__OneNodeAfterOperation)
{
    auto dll = createDllOnHeap(2);
    auto s = dll_status_nok;
    auto nh = dll_node_delete_end(dll, &s, getSimpleNodeDecayFn());
    CHECK_EQUAL(dll_status_ok, s);
    POINTERS_EQUAL(nullptr, dll->next->next);
    destroyDllOnHeap(nh);
}

TEST(Dll, dll_traverse__HeadNodeOrTraverseFnNull__NothingIsDone)
{
    dll_traverse(nullptr, [](const dll_node* node, void* ud){}, nullptr);
    auto dll = createDllNode();
    dll_traverse(&dll, nullptr, nullptr);
    /* The test should not crash the program */
}

TEST(Dll, dll_traverse__TenNodes__CalcTheNumberOfNodes__TenReturned)
{
    size num = 0;
    auto list = createDllOnHeap(9);
    dll_traverse(list, [](const dll_node* node, void* user_data) {
        auto ctr = static_cast<size*>(user_data);
        *ctr += 1;
    }, &num);
    CHECK_EQUAL(10, num);
    destroyDllOnHeap(list);
}

TEST(Dll, dll_node_count__NullPassed__ZeroReceived)
{
    CHECK_EQUAL(0, dll_node_count(nullptr));
}

TEST(Dll, dll_node_count__TenNodes__TenReturned)
{
    auto lst = createDllOnHeap(9);
    CHECK_EQUAL(10, dll_node_count(lst));
    destroyDllOnHeap(lst);
}

TEST(Dll, dll_node_find__HeadIsNullOrCmpFnIsNull__NoError)
{
    auto list = createDllNode();
    auto cmpFn = [](const void* ud) {
        return true;
    };
    POINTERS_EQUAL(nullptr, dll_node_find(nullptr, cmpFn));
    POINTERS_EQUAL(nullptr, dll_node_find(&list, nullptr));
}

TEST(Dll, dll_node_find__SingleNode__TheSameNodeReturned)
{
    auto list = createDllNode();
    auto nodeFound = dll_node_find(&list, getSimpleNodeCmpFn());
    POINTERS_EQUAL(&list, nodeFound);
}

TEST(Dll, dll_node_find__MultipleNodesWithTheSameData__FirstOccurrenceIsReturned)
{
    auto list = createDllOnHeap(10);
    POINTERS_EQUAL(list, dll_node_find(list, getSimpleNodeCmpFn()));
    destroyDllOnHeap(list);
}

TEST(Dll, dll_node_find__SpecificNodeFound)
{
    const u32 newMagicNumber = 0xBABA;
    UserData newUserData = {
        .magicNumber = newMagicNumber
    };

    auto list = createDllOnHeap(3);
    auto tail = list->next->next->next;
    /* Change user data of the last node */
    tail->user_data = &newUserData;
    POINTERS_EQUAL(tail, dll_node_find(list, [](const void* user_data) {
        auto userDataPtr = static_cast<const UserData*>(user_data);
        return (userDataPtr->magicNumber == newMagicNumber);
    }));
    destroyDllOnHeap(list);
}

TEST(Dll, dll_node_find__InvalidMagicNumber__NothingFound)
{
    auto list = createDllOnHeap(9);
    POINTERS_EQUAL(nullptr, dll_node_find(list, [](const void* user_data) {
        return (static_cast<const UserData*>(user_data)->magicNumber == MAGIC_NUMBER + 1);
    }));
    destroyDllOnHeap(list);
}

TEST(Dll, dll_node_find__TraverseUntilCertainConditionIsMet)
{
    const size numOfNodes = 3;
    u8 userData[numOfNodes] = {0xAA, 0xBB, 0xCC};
    /* The function takes the number of extra nodes, thus subtracting one is mandatory */
    auto list = createDllOnHeap(numOfNodes - 1);
    auto head = list;
    head->user_data = &userData[0];
    head->next->user_data = &userData[1];
    head->next->next->user_data = &userData[2];

    /* Traverse until user data equals 0xCC */
    auto node = dll_node_find(head, [](const void *user_data) {
        return (*(static_cast<const u8*>(user_data)) == 0xCC);
    });
    POINTERS_EQUAL(head->next->next, node);
    destroyDllOnHeap(list);
}