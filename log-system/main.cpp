#include "../log/log.h"
#include <unistd.h>

Log *log = Log::getInstance(5, 512, 64);

void func() {
    char str[100] = "hello\n";
    while (true) {
        log->append(str, LogLevel::INFO);
        log->append(str, LogLevel::DEBUG);
        log->append(str, LogLevel::WARN);
        log->append(str, LogLevel::ERROR);
        log->append(str, LogLevel::FATAL);
        sleep(1);
    }
}

int main() {
    std::thread t1(func);
    std::thread t2(func);
    std::thread t3(func);
    std::thread t4(func);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    return 0;
}