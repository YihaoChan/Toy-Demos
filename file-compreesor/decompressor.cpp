#include "include/decompressor.hpp"
#include <string.h>
#include "windows.h"

#define CHECK_FILENAME(filename)\
    do {\
        WIN32_FIND_DATA FindFileData;\
        HANDLE hFind;\
        hFind = FindFirstFile(filename, &FindFileData);\
        if (INVALID_HANDLE_VALUE == hFind) {\
            fprintf(stderr, "error filename: %s\n", filename);\
            exit(1);\
        }\
    } while (0)

void usage();

int main(int argc, char **argv) {
    if (argc != 2) {
        usage();
        return 1;
    }
    string fileName = argv[1];
    CHECK_FILENAME(fileName.c_str());
    const char *temp = fileName.c_str();
    const char *postfix = strrchr(temp, '.') + 1;
    if (strcmp(postfix, "huffman") != 0) {
        printf("[FATAL] Only support .huffman!\n");
        return 1;
    }
    Decompressor decompressor(fileName);
    decompressor.decompress();
    printf("[FINISHED] The decompressed file is generated in the directory identical to the file to decompress.\n");
}

void usage() {
    printf("[USAGE] decompressor [file_to_decompress]\n");
}