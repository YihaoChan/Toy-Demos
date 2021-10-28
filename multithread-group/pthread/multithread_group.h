#pragma once

#include <pthread.h>
#include <vector>
#include <cstdio>
#include <cstdlib>

using std::vector;

class MultiThreadGroup {
public:
    MultiThreadGroup(vector<int> &nums, vector<pthread_t>& threads);

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

    static void *threadFunc(void *args);

    void sortPerThread(Args *args);

    vector<int> &m_nums;
    vector<Args *> m_pArgs;
    int m_frontThreadNum; // ǰ�沿�ֵ��̸߳���
    int m_frontSortLen; // ǰ��n-1���߳��У�һ���̸߳��𼸸���
    int m_backSortLen; // ���һ���̸߳��𼸸���
    vector<pthread_t> &m_threads;
};

extern pthread_barrier_t barrier;