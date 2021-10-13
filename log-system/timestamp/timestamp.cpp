#include "timestamp.h"

Timestamp::Timestamp() {}

Timestamp::~Timestamp() {}

struct tm Timestamp::getTm() {
    struct timeval tv;
    struct tm time;
    gettimeofday(&tv, NULL); // 获取微秒、秒
    localtime_r(&tv.tv_sec, &time); // 将s转换为tm格式
    time.tm_year += 1900; // 返回值是从1900年算起到现在多少年，所以加上1900就是今年
    time.tm_mon += 1; // 返回值范围为0-11月，要加1
    return time;
}

// 时间转为字符串，精确到秒
std::string Timestamp::timeUntilSec() {
    struct tm time;
    bzero(m_str, sizeof(m_str));
    time = getTm();
    snprintf(m_str, sizeof(m_str),
             "%d-%d-%d %d:%d:%d",
             time.tm_year,
             time.tm_mon,
             time.tm_mday,
             time.tm_hour,
             time.tm_min,
             time.tm_sec);
    return m_str;
}

// 时间转为字符串，精确到天
std::string Timestamp::timeUntilDay() {
    struct tm time;
    bzero(m_str, sizeof(m_str));
    time = getTm();
    snprintf(m_str, sizeof(m_str),
             "%d-%d-%d",
             time.tm_year,
             time.tm_mon,
             time.tm_mday);
    return m_str;
}