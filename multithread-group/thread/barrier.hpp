#pragma once

#include <condition_variable>
#include <thread>
#include <chrono>
#include <mutex>

using std::condition_variable;
using std::mutex;
using std::thread;
using std::unique_lock;

class Barrier {
public:
    explicit Barrier(int threadNum) : m_barrierCount(threadNum),
                                      m_release(0) {}

    void setBarrierCount(int barrierCount) {
        m_barrierCount = barrierCount;
    }

    void waitAndNotify() {
        unique_lock<mutex> uniqueLock(m_lock);
        --m_barrierCount;
        if (0 == m_barrierCount) {
            m_release = 1;
            m_cv.notify_all();
        } else {
            m_cv.wait(uniqueLock, [&] { return m_release == 1; });
        }
    }

private:
    mutex m_lock;
    condition_variable m_cv;
    unsigned int m_barrierCount;
    unsigned int m_release;
};