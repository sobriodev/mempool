#include "TestRunner.h"
#include "dll.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

TEST_GROUP(DllSanityCheck)
{
    void setup() override {}
    void teardown() override {}

    static auto createNode()
    {
        auto node = new dll_node;
        CHECK_EQUAL(dll_status_ok, dll_node_create(node, nullptr));
        return node;
    }

    static auto createListOfTwoNodes()
    {
        auto head = createNode();
        auto tail = createNode();
        auto status = dll_status_nok;
        CHECK_EQUAL(head, dll_node_insert_after(head, tail, &status));
        CHECK_EQUAL(dll_status_ok, status);
        return head;
    }

    static void destroyList(dll_node* head)
    {
        CHECK_EQUAL(dll_status_ok, dll_destroy(head, [](dll_node* node) {
            delete node;
        }));
    }
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(DllSanityCheck, dll_node_insert_before__EntireListInsteadOfSingleNode__InvalidNodeError)
{
    auto list = createListOfTwoNodes();
    auto list2 = createListOfTwoNodes();
    auto status = dll_status_ok;
    dll_node_insert_before(list, list2, &status);
    CHECK_EQUAL(dll_status_inv_node, status);
    destroyList(list);
    destroyList(list2);
}

TEST(DllSanityCheck, dll_node_insert_after__EntireListInsteadOfSingleNode__InvalidNodeError)
{
    auto list = createListOfTwoNodes();
    auto list2 = createListOfTwoNodes();
    auto status = dll_status_ok;
    dll_node_insert_after(list, list2, &status);
    CHECK_EQUAL(dll_status_inv_node, status);
    destroyList(list);
    destroyList(list2);
}

TEST(DllSanityCheck, dll_destroy__PassTailNode__ExpectError)
{
    auto list = createListOfTwoNodes();
    CHECK_EQUAL(dll_status_inv_node, dll_destroy(list->next, [](dll_node *n) {}));
    destroyList(list);
}