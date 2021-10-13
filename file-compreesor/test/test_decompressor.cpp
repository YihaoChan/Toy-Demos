#include "../include/decompressor.hpp"
#include "windows.h"
#include <stdlib.h>
#include <direct.h>

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

static void test_decompressor_1() {
    string fileName = "./sample/7a5b2c4d.txt.huffman";
    CHECK_FILENAME(fileName.c_str());
    Decompressor decompressor(fileName);
    decompressor.decompress();
    _chdir("./sample");
    system("fc 7a5b2c4d.txt 7a5b2c4d.txt.decompress");
    _chdir("..");
}

static void test_decompressor_2() {
    string fileName = "./sample/1a2b3c4d.txt.huffman";
    CHECK_FILENAME(fileName.c_str());
    Decompressor decompressor(fileName);
    decompressor.decompress();
    _chdir("./sample");
    system("fc 1a2b3c4d.txt 1a2b3c4d.txt.decompress");
    _chdir("..");
}

static void test_decompressor_3() {
    string fileName = "./sample/chinese.txt.huffman";
    CHECK_FILENAME(fileName.c_str());
    Decompressor decompressor(fileName);
    decompressor.decompress();
    _chdir("./sample");
    system("fc chinese.txt chinese.txt.decompress");
    _chdir("..");
}

static void test() {
    test_decompressor_1();
    test_decompressor_2();
    test_decompressor_3();
}

int main() {
    test();
    return 0;
}