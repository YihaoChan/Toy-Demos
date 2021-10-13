#ifndef LOG_FILE_H_
#define LOG_FILE_H_

#include <string>
#include <queue>
#include "../timestamp/timestamp.h"

using std::string;
using std::queue;

class RollFile {
public:
    RollFile(int maxFileNum, int maxFileSize);

    ~RollFile();

    // 添加日志，并实现滚动日志文件
    void write2file(char *log, int len);

private:
    queue <string> m_fileNameQueue; // 日志文件名队列，用于压缩最旧的文件
    int m_maxFileNum; // 最大文件数量，当文件数量超过这个数量时，压缩最旧的文件
    int m_maxFileSize; // 文件到多大时需要滚动
    int m_fd; // 文件描述符
    int m_currFileSize; // 当前文件大小
    int m_currFileNum; // 已有文件个数
    int setupFile(const char *fileName); // 创建文件
    string m_dir; // 日志文件存放目录
    string m_file; // 日志文件名
    string m_fullPath; // 日志文件完整路径
    Timestamp m_timestamp; // 时间戳对象
};

#endif

