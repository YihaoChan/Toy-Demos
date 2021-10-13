#include "log.h"
#include <stdio.h>
#include <functional>
#include <iostream>
#include <sstream>
#include <cassert>

using std::unique_lock;
using std::stringstream;

Log *Log::getInstance(int maxFileNum, int maxFileSize, int bufferSize) {
    // C++11以后，局部静态变量保证是线程安全的
    static Log log(maxFileNum, maxFileSize, bufferSize);
    return &log;
}

Log::Log(int maxFileNum, int maxFileSize, int bufferSize) : m_maxFileNum(maxFileNum),
                                                            m_bufferSize(bufferSize),
                                                            m_maxFileSize(maxFileSize),
                                                            m_isRunning(true),
                                                            m_currBuffer(new Buffer(bufferSize)),
                                                            m_auxBuffer(new Buffer(bufferSize)),
                                                            m_acceptThread(std::bind(&Log::threadFunc, this)) {
    /**
     * 1.bind绑定一个类的成员函数，其第一个参数必须是该类型的一个对象，即bind(&成员函数，第一个参数，第二个参数，第三个参数...)
     *   bind方法相当于将传参的自由度提高，传多少参数都可以，它会自己检验、对准参数。
     * 2.
     *   2.1 m_acceptThread是一个std::thread对象，这个对象绑定了threadFunc回调函数。
     *       当std::thread对象m_acceptThread被创建的时候，这个回调函数就被执行，类似于传统线程库中pthread_create时调用回调函数。
     *       然后线程就进入threadFunc()函数，这个函数while (running)的时候一直在做buffer -> 磁盘的工作。
     *   2.2 因此，整个后台线程的完整过程就是：Log对象创建的时候，这个线程就调用回调函数，然后一直在后台执行写盘的工作。
     *       如果遇到缓冲区还没创建好，即可写的字节数小于传入长度，它就阻塞在条件变量上。
     *       等缓冲区准备好了的时候，它就被唤醒继续执行上述工作。
     *   2.3 这样，就完成了【前台线程调用append方法加日志 -> 然后写到缓冲区上 -> 后台线程从缓冲区写盘】的完整流程。
     */
}

// 添加日志行
void Log::append(char *logLine, LogLevel level) {
    // 操作公有缓冲区，要加锁
    unique_lock <mutex> uniqueLock(m_mutex);
    // 日志信息与时间、线程ID、级别等拼接
    string content = logLine;
    string timestamp = m_timestamp.timeUntilSec();
    thread::id tid = std::this_thread::get_id();
    stringstream ss;
    ss << tid;
    string threadID = ss.str();
    string logLevel = level2str(level);
    string allInfo = timestamp + " " + logLevel + " " + "[thread:" + threadID + "] " + content;
    // 前台线程写入缓冲区，后台线程一直在执行适时写盘操作写到磁盘文件中
    write2buffer(allInfo.c_str(), allInfo.length());
    return;
}

// 将日志内容写入buffer中
void Log::write2buffer(const char *logLine, int len) {
    // 如果当前buffer空间足够，就直接写入缓冲区
    if (m_currBuffer->getWritableSize() >= len) {
        m_currBuffer->append(logLine, len);
    } else {
        // 移动语义，使得无需拷贝，然后m_currBuffer被置空
        m_buffers.push_back(std::move(m_currBuffer));

        if (nullptr != m_auxBuffer) {
            // 将辅助缓冲区的控制权交给主缓冲区
            m_currBuffer = std::move(m_auxBuffer);
        } else {
            // 申请一块新的buffer，让主缓冲区指向这块buffer，然后新buffer被delete
            m_currBuffer.reset(new Buffer(m_bufferSize));
        }
        // 以上步骤相当于创建了一个新缓冲区
        m_currBuffer->append(logLine, len);
        // 唤醒后台线程写入磁盘
        m_cond.notify_one();
    }
    return;
}

// 将对应的level转化为字符串
string Log::level2str(LogLevel level) {
    switch (level) {
        case LogLevel::FATAL:
            return string("FATAL");
        case LogLevel::ERROR:
            return string("ERROR");
        case LogLevel::WARN:
            return string("WARN");
        case LogLevel::INFO:
            return string("INFO");
        case LogLevel::DEBUG:
            return string("DEBUG");
        default:
            return string("UNKNOWN");
    }
    return nullptr;
}

// 线程函数
void Log::threadFunc() {
    unique_ptr <Buffer> newBuffer1(new Buffer(m_bufferSize));
    unique_ptr <Buffer> newBuffer2(new Buffer(m_bufferSize));
    vector <unique_ptr<Buffer>> buffersToWrite; // 暂存内容的缓冲区，准备写到磁盘
    RollFile rollFile(m_maxFileNum, m_maxFileSize);
    buffersToWrite.reserve(16); // 预留空间

    // 线程一直在适时写盘
    while (m_isRunning) {
        /*** 代码块，利用RAII机制，加锁以后在离开代码块时，锁对象析构，从而自动解锁 ***/
        {
            /**
             * 在管理互斥锁的时候，使用的是unique_lock而不是lock_guard，而且事实上也不能使用lock_guard。
             * 这需要先解释下wait()函数所做的事情，可以看到，在wait()函数之前，使用互斥锁保护了，
             * 如果wait的时候什么都没做，岂不是一直持有互斥锁？那生产者也会一直卡住，不能够将数据放入队列中了。
             * 所以，wait()函数会先调用互斥锁的unlock()函数，然后再将自己睡眠，在被唤醒后，又会继续持有锁，保护后面的队列操作。
             * lock_guard没有lock和unlock接口，而unique_lock提供了，这就是必须使用unique_lock的原因。
             */
            unique_lock <mutex> uniqueLock(m_mutex);
            // 如果没有缓冲区准备好，当前线程就阻塞，直到buffer准备好
            while (m_buffers.empty()) {
                /**
                 * 在判断队列是否为空的时候，使用的是while(q.empty())，而不是if(q.empty())。
                 * 这是因为wait()从阻塞到返回，不一定就是由于notify_one()函数造成的，还有可能由于系统的不确定原因唤醒，
                 * 这个的时机和频率都是不确定的，被称作伪唤醒。
                 * 如果在错误的时候被唤醒了，队列并不一定非空，所以需要再次判断队列是否为空。如果还是为空，就继续wait()阻塞。
                 */
                m_cond.wait_for(uniqueLock, std::chrono::seconds(3));
            }
            m_buffers.push_back(std::move(m_currBuffer));
            /**
             * 后面会调用reset方法重置这个new出来的缓冲区，
             * 如果在reset方法中加shrink方法释放所有空间，那么这块空间就变成什么都没有，此时move就会崩溃。
             * 所以，reset方法只能置零几个缓冲区指针。
             */
            m_currBuffer = std::move(newBuffer1);
            // 把buffers交换给待写缓冲区
            buffersToWrite.swap(m_buffers);
            // 一直保证辅助缓冲区非空
            if (nullptr == m_auxBuffer) {
                m_auxBuffer = std::move(newBuffer2);
            }
        }
        assert(!buffersToWrite.empty());

        // 将buffer中的内容写进文件中
        for (int i = 0; i < buffersToWrite.size(); i++) {
            unique_lock <mutex> uniqueLock(m_mutex);
            rollFile.write2file(buffersToWrite[i]->getReadPtr(), buffersToWrite[i]->getReadableSize());
        }

        if (buffersToWrite.size() > 2) {
            buffersToWrite.resize(2);
        }

        // 通过资源转移保证newBuffer1和newBuffer2一直非空，提升利用率
        if (nullptr == newBuffer1) {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite[0]);
            newBuffer1->resetBuffer(); // 重置buffer，相当于初始化
        }
        if (nullptr == newBuffer2) {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite[1]);
            newBuffer2->resetBuffer(); // 重置buffer，相当于初始化
        }

        buffersToWrite.clear();
    }

    return;
}

// 关闭日志系统
void Log::stop() {
    if (m_isRunning) {
        m_isRunning = false;
        m_acceptThread.join(); // 等待后台线程退出
        printf("Log system closed.\n");
    }
}

Log::~Log() {
    stop();
}