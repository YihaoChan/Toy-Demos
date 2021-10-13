#ifndef LOG_H_
#define LOG_H_

#include <memory>
#include "../buffer/buffer.h"
#include "../timestamp/timestamp.h"
#include "../roll_file/roll_file.h"
#include <vector>
#include <thread>
#include <string>
#include <mutex>
#include <condition_variable>

using std::string;
using std::vector;
using std::unique_ptr;
using std::thread;
using std::mutex;
using std::condition_variable;

enum LogLevel {
    FATAL,  // 导致程序退出的致命错误
    ERROR,  // 发生了错误但不影响系统运行
    WARN,   // 警告
    INFO,   // 正常信息
    DEBUG   // 调试程序有关的信息
};

class Log {
public:
    static Log *getInstance(int maxFileNum, int maxFileSize, int m_bufferSize); // 单例

    void append(char *logLine, LogLevel level); // 添加日志行
    void stop(); // 关闭日志系统
private:
    Log(int maxFileNum, int maxFileSize, int m_bufferSize);

    // 既然日志系统是单例，肯定不允许外面调用构造函数实例化新对象；也不允许拷贝间接实例化新对象；也不允许对象赋值。
    Log() = default; // 声明带参数的构造函数之余也要生成默认构造函数，以免破坏POD类型
    ~Log();

    Log(const Log &log) = delete; // 不允许拷贝构造
    Log &operator=(const Log &log) = delete; // 不允许赋值

    void threadFunc(); // 线程函数
    void write2buffer(const char *logLine, int len); // 将日志内容写入buffer中
    string level2str(LogLevel level); // 将对应的level转化为字符串

    Timestamp m_timestamp; // 时间戳
    int m_maxFileSize; // 文件达到多大时滚动
    int m_maxFileNum; // 最大文件数量，当文件数大于这个数量时，压缩最旧的那个文件
    int m_bufferSize; // 缓冲区大小
    bool m_isRunning; // 日志系统是否正在运行
    unique_ptr <Buffer> m_currBuffer; // 主缓冲区
    unique_ptr <Buffer> m_auxBuffer; // 辅助缓冲区
    vector <unique_ptr<Buffer>> m_buffers; // 保存buffer指针的vector
    thread m_acceptThread; // 后台接受数据的线程
    mutex m_mutex; // 互斥变量
    condition_variable m_cond; // 条件变量
};

#endif
