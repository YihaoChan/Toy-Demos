#include "../include/huffman_tree.hpp"
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

static void test_huffman_tree_1() {
    map<string, int> char2freq = {
            {"a", 7},
            {"b", 5},
            {"c", 2},
            {"d", 4}
    };
    HuffmanTree huffmanTree(char2freq);
    huffmanTree.build();
    TreeNode *root = huffmanTree.getRoot();
    EXPECT_STREQ(root->m_data.c_str(), "");
    EXPECT_EQ(root->m_weight, 18, "%d");
    EXPECT_EQ(root->m_isLeaf, false, "%d");
    EXPECT_STREQ(root->m_left->m_data.c_str(), "a");
    EXPECT_EQ(root->m_left->m_weight, 7, "%d");
    EXPECT_EQ(root->m_left->m_isLeaf, true, "%d");
    EXPECT_STREQ(root->m_right->m_data.c_str(), "");
    EXPECT_EQ(root->m_right->m_weight, 11, "%d");
    EXPECT_EQ(root->m_right->m_isLeaf, false, "%d");
    /**
     * 返回的右值本来在表达式语句结束后，其生命也就该终结了(因为是临时变量)，而通过右值引用，该右值又重获新生，
     * 其生命期将与右值引用类型变量的生命期一样，只要右值引用变量还存活，该右值临时变量将会一直存活下去。
     * 因此，不会调用拷贝构造函数，节省时间和空间。
     * 而不能在函数中创建临时对象后返回它的引用，因为当函数结束时，临时对象将消失。
     */
    map<string, string> &&char2code = huffmanTree.encode();
    EXPECT_STREQ(char2code["a"].c_str(), "0");
    EXPECT_STREQ(char2code["b"].c_str(), "10");
    EXPECT_STREQ(char2code["c"].c_str(), "110");
    EXPECT_STREQ(char2code["d"].c_str(), "111");
}

static void test_huffman_tree_2() {
    map<string, int> char2freq = {
            {"a", 1},
            {"b", 2},
            {"c", 3},
            {"d", 4}
    };
    HuffmanTree huffmanTree(char2freq);
    huffmanTree.build();
    TreeNode *root = huffmanTree.getRoot();
    EXPECT_STREQ(root->m_data.c_str(), "");
    EXPECT_EQ(root->m_weight, 10, "%d");
    EXPECT_EQ(root->m_isLeaf, false, "%d");
    EXPECT_STREQ(root->m_left->m_data.c_str(), "d");
    EXPECT_EQ(root->m_left->m_weight, 4, "%d");
    EXPECT_EQ(root->m_left->m_isLeaf, true, "%d");
    EXPECT_STREQ(root->m_right->m_data.c_str(), "");
    EXPECT_EQ(root->m_right->m_weight, 6, "%d");
    EXPECT_EQ(root->m_right->m_isLeaf, false, "%d");
    map<string, string> &&char2code = huffmanTree.encode();
    EXPECT_STREQ(char2code["a"].c_str(), "100");
    EXPECT_STREQ(char2code["b"].c_str(), "101");
    EXPECT_STREQ(char2code["c"].c_str(), "11");
    EXPECT_STREQ(char2code["d"].c_str(), "0");
}

static void test_huffman_tree_3() {
    map<string, int> char2freq = {{"a", 1}};
    HuffmanTree huffmanTree(char2freq);
    huffmanTree.build();
    TreeNode *root = huffmanTree.getRoot();
    EXPECT_STREQ(root->m_data.c_str(), "a");
    EXPECT_EQ(root->m_weight, 1, "%d");
    EXPECT_EQ(root->m_isLeaf, true, "%d");
    map<string, string> &&char2code = huffmanTree.encode();
    EXPECT_STREQ(char2code["a"].c_str(), "");
}

static void test_huffman_tree_4() {
    map<string, int> char2freq = {
            {"a", 1},
            {"b", 2}
    };
    HuffmanTree huffmanTree(char2freq);
    huffmanTree.build();
    TreeNode *root = huffmanTree.getRoot();
    EXPECT_STREQ(root->m_data.c_str(), "");
    EXPECT_EQ(root->m_weight, 3, "%d");
    EXPECT_EQ(root->m_isLeaf, false, "%d");
    EXPECT_STREQ(root->m_left->m_data.c_str(), "a");
    EXPECT_EQ(root->m_left->m_weight, 1, "%d");
    EXPECT_EQ(root->m_left->m_isLeaf, true, "%d");
    EXPECT_STREQ(root->m_right->m_data.c_str(), "b");
    EXPECT_EQ(root->m_right->m_weight, 2, "%d");
    EXPECT_EQ(root->m_right->m_isLeaf, true, "%d");
    map<string, string> &&char2code = huffmanTree.encode();
    EXPECT_STREQ(char2code["a"].c_str(), "0");
    EXPECT_STREQ(char2code["b"].c_str(), "1");
}

static void test_huffman_tree_5() {
    map<string, int> char2freq;
    HuffmanTree huffmanTree(char2freq);
    huffmanTree.build();
    TreeNode *root = huffmanTree.getRoot();
    EXPECT_EQ(root, nullptr, "%p");
}

static void test() {
    test_huffman_tree_1();
    test_huffman_tree_2();
    test_huffman_tree_3();
    test_huffman_tree_4();
    test_huffman_tree_5();
}

int main() {
    test();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}