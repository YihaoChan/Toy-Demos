#pragma once

#include <pthread.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include "barrier.hpp"

using std::vector;

class MultiThreadGroup {
public:
    MultiThreadGroup(vector<int> &nums, Barrier &barrier, vector<thread> &threads);

    ~MultiThreadGroup();

    void group();

    int getThreadNum() const; // 返回分配之后的线程个数
    int getPerGroupLen() const; // 返回每个线程负责的元素个数

private:
    struct Args {
        Args(vector<int> &nums_, int start_, int end_) : nums(nums_), start(start_), end(end_) {}

        vector<int> &nums;
        int start;
        int end;
    };

    void dispatchNumsForThreads(int threadNum); // 为每个线程分配负责的元素个数

    int partition(vector<int> &nums, int low, int high);

    void quickSort(vector<int> &nums, int low, int high);

    void threadFunc(Args *args);

    vector<int> &m_nums;
    vector<Args *> m_pArgs;

    int m_frontThreadNum; // 前面部分的线程个数
    int m_frontSortLen; // 前面n-1个线程中，一个线程负责几个数
    int m_backSortLen; // 最后一个线程负责几个数
    vector<thread> &m_threads;
    Barrier &m_barrier;
};

