#include "../log/log.h"
#include <iostream>

using std::cout;
using std::endl;

int main() {
    Log *log = Log::getInstance(3, 512, 64);
    Log *log2 = Log::getInstance(3, 512, 64);
    cout << (void *) log << "\t" << (void *) log2 << endl;
    char str[100] = "hello\n";
    for (int i = 0; i < 20; ++i) {
        log->append(str, LogLevel::INFO);
        log->append(str, LogLevel::DEBUG);
        log->append(str, LogLevel::WARN);
        log->append(str, LogLevel::ERROR);
        log->append(str, LogLevel::FATAL);
    }
    log->stop();
    return 0;
}