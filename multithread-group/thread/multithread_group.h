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

    int getThreadNum() const; // ���ط���֮����̸߳���
    int getPerGroupLen() const; // ����ÿ���̸߳����Ԫ�ظ���

private:
    struct Args {
        Args(vector<int> &nums_, int start_, int end_) : nums(nums_), start(start_), end(end_) {}

        vector<int> &nums;
        int start;
        int end;
    };

    void dispatchNumsForThreads(int threadNum); // Ϊÿ���̷߳��为���Ԫ�ظ���

    int partition(vector<int> &nums, int low, int high);

    void quickSort(vector<int> &nums, int low, int high);

    void threadFunc(Args *args);

    vector<int> &m_nums;
    vector<Args *> m_pArgs;

    int m_frontThreadNum; // ǰ�沿�ֵ��̸߳���
    int m_frontSortLen; // ǰ��n-1���߳��У�һ���̸߳��𼸸���
    int m_backSortLen; // ���һ���̸߳��𼸸���
    vector<thread> &m_threads;
    Barrier &m_barrier;
};

