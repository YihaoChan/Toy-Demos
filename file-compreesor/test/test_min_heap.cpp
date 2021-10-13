#include "../include/min_heap.hpp"
#include <cstring>

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ(actual, expect, format) \
    do {\
        ++test_count;\
        if ((actual) == (expect)) {\
            ++test_pass;\
        } else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", \
            __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_STREQ(actual, expect) \
    do {\
        ++test_count;\
        if (!(strcmp(actual, expect))) {\
            ++test_pass;\
        } else {\
        fprintf(stderr, "%s:%d: expect: %s actual: %s\n", \
        __FILE__, __LINE__, expect, actual);\
        main_ret = 1;\
        }\
    } while(0)

static void test_min_heap_1() {
    MinHeap minHeap;
    minHeap.push(new TreeNode("a", 7, nullptr, nullptr, true));
    minHeap.push(new TreeNode("b", 5, nullptr, nullptr, true));
    minHeap.push(new TreeNode("c", 2, nullptr, nullptr, true));
    minHeap.push(new TreeNode("d", 4, nullptr, nullptr, true));

    minHeap.buildHeap();

    EXPECT_STREQ(minHeap.top()->m_data.c_str(), "c");
    EXPECT_EQ(minHeap.top()->m_weight, 2, "%d");
    minHeap.pop();

    EXPECT_STREQ(minHeap.top()->m_data.c_str(), "d");
    EXPECT_EQ(minHeap.top()->m_weight, 4, "%d");
    minHeap.pop();

    EXPECT_STREQ(minHeap.top()->m_data.c_str(), "b");
    EXPECT_EQ(minHeap.top()->m_weight, 5, "%d");
    minHeap.pop();

    EXPECT_STREQ(minHeap.top()->m_data.c_str(), "a");
    EXPECT_EQ(minHeap.top()->m_weight, 7, "%d");
    minHeap.pop();

    EXPECT_EQ(minHeap.top(), nullptr, "%p");
}

static void test_min_heap_2() {
    MinHeap minHeap;
    minHeap.push(new TreeNode("a", 1, nullptr, nullptr, true));

    minHeap.buildHeap();

    EXPECT_STREQ(minHeap.top()->m_data.c_str(), "a");
    EXPECT_EQ(minHeap.top()->m_weight, 1, "%d");
    minHeap.pop();

    EXPECT_EQ(minHeap.top(), nullptr, "%p");
}

static void test_min_heap_3() {
    MinHeap minHeap;
    minHeap.push(new TreeNode("a", 1, nullptr, nullptr, true));
    minHeap.push(new TreeNode("b", 2, nullptr, nullptr, true));

    minHeap.buildHeap();

    EXPECT_STREQ(minHeap.top()->m_data.c_str(), "a");
    EXPECT_EQ(minHeap.top()->m_weight, 1, "%d");
    minHeap.pop();

    EXPECT_STREQ(minHeap.top()->m_data.c_str(), "b");
    EXPECT_EQ(minHeap.top()->m_weight, 2, "%d");
    minHeap.pop();

    EXPECT_EQ(minHeap.top(), nullptr, "%p");
}

static void test_min_heap_4() {
    MinHeap minHeap;
    minHeap.push(nullptr);

    minHeap.buildHeap();

    EXPECT_EQ(minHeap.top(), nullptr, "%p");
}

static void test() {
    test_min_heap_1();
    test_min_heap_2();
    test_min_heap_3();
    test_min_heap_4();
}

int main() {
    test();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}