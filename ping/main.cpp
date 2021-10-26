#include "ping.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        usage();
        exit(1);
    }
    Ping ping(argv[1]);
    ping.run();
    return 0;
}