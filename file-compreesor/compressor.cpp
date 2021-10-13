#include "include/compressor.hpp"
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
    if (strcmp(postfix, "txt") != 0) {
        printf("[FATAL] Only support .txt!\n");
        return 1;
    }
    Compressor compressor(fileName);
    compressor.compress();
    printf("[FINISHED] The compressed file is generated in the directory identical to the file to compress.\n");
}

void usage() {
    printf("[USAGE] compressor [file_to_compress]\n");
}