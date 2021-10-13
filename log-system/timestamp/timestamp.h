#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using std::string;

class Timestamp {
public:
    Timestamp();

    ~Timestamp();

    // 获取当前tm格式的时间
    struct tm getTm();

    // 时间转为字符串，精确到秒
    string timeUntilSec();

    // 时间转为字符串，精确到天
    string timeUntilDay();

private:
    char m_str[200];
};

#endif
