#include "../include/compressor.hpp"

#include "windows.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define CHECK_FILENAME(filename)\
    do {\
        WIN32_FIND_DATA FindFileData;\
        HANDLE hFind;\
        hFind = FindFirstFile(filename, &FindFileData);\
        if (INVALID_HANDLE_VALUE == hFind) {\
            fprintf(stderr, "%s:%d: error filename: %s\n",\
            __FILE__, __LINE__, filename);\
            exit(1);\
        }\
    } while (0)

#define EXPECT_EQ(actual, expect, format)\
    do {\
        ++test_count;\
        if ((actual) == (expect)) {\
            ++test_pass;\
        } else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n",\
            __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

static void test_compressor_1() {
    string fileName = "./sample/7a5b2c4d.txt";
    CHECK_FILENAME(fileName.c_str());
    Compressor compressor(fileName);
    compressor.compress();
    vector<char> &&chars = compressor.getCodes();
    EXPECT_EQ((int) chars.size(), 5, "%d");
    EXPECT_EQ((unsigned char) (chars[0]), 1, "%d");
    EXPECT_EQ((unsigned char) (chars[1]), 85, "%d");
    EXPECT_EQ((unsigned char) (chars[2]), 109, "%d");
    EXPECT_EQ((unsigned char) (chars[3]), 255, "%d");
    EXPECT_EQ((unsigned char) (chars[4]), 224, "%d");
}

static void test_compressor_2() {
    string fileName = "./sample/1a2b3c4d.txt";
    CHECK_FILENAME(fileName.c_str());
    Compressor compressor(fileName);
    compressor.compress();
    vector<char> &&chars = compressor.getCodes();
    EXPECT_EQ((int) chars.size(), 3, "%d");
    EXPECT_EQ((unsigned char) (chars[0]), 150, "%d");
    EXPECT_EQ((unsigned char) (chars[1]), 254, "%d");
    EXPECT_EQ((unsigned char) (chars[2]), 0, "%d");
}

static void test_compressor_3() {
    string fileName = "./sample/chinese.txt";
    CHECK_FILENAME(fileName.c_str());
    Compressor compressor(fileName);
    compressor.compress();
    vector<char> &&chars = compressor.getCodes();
    EXPECT_EQ((int) chars.size(), 5, "%d");
    EXPECT_EQ((unsigned char) (chars[0]), 1, "%d");
    EXPECT_EQ((unsigned char) (chars[1]), 85, "%d");
    EXPECT_EQ((unsigned char) (chars[2]), 109, "%d");
    EXPECT_EQ((unsigned char) (chars[3]), 255, "%d");
    EXPECT_EQ((unsigned char) (chars[4]), 224, "%d");
}

static void test() {
    test_compressor_1();
    test_compressor_2();
    test_compressor_3();
}

int main() {
    test();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
