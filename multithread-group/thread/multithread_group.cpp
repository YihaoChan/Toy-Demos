#include <functional>
#include <thread>
#include "multithread_group.h"

using std::thread;

MultiThreadGroup::MultiThreadGroup(vector<int> &nums, Barrier &barrier, vector<thread> &threads) :
        m_nums(nums),
        m_frontThreadNum(0),
        m_frontSortLen(0),
        m_backSortLen(0),
        m_barrier(barrier),
        m_threads(threads) {
    // 将线程个数调整至合适，并为每个线程分配负责的元素个数
    dispatchNumsForThreads(int(threads.size()));
}

void MultiThreadGroup::dispatchNumsForThreads(int threadNum) {
    if (threadNum < 0) {
        printf("ThreadNum should not be negative!\n");
        exit(1);
    }

    // 线程个数不要超过数组元素的一半，否则会有一部分线程只排1个元素，相当于没操作，反倒花费线程开销
    if (((0 == (int(m_nums.size()) % 2)) && (threadNum > (int(m_nums.size()) / 2))) ||
        ((1 == (int(m_nums.size()) % 2)) && (threadNum > (int(m_nums.size()) / 2 + 1)))) {
        threadNum = int(m_nums.size()) / 2;
    }

    // 有可能元素个数为1，那么threadNum = m_nums.size() / 2就变为0，下面就会出现除以0的情况
    if (0 == threadNum) {
        threadNum = 1;
    }

    // nums.size() / threadNum = a ...... b
    int quotient = int(m_nums.size()) / threadNum; // a

    // 前面threadNum-1个线程中每个线程负责a个元素，最后一个线程负责b个元素
    m_frontThreadNum = threadNum - 1;
    m_frontSortLen = quotient;
    // 如果可以整除，那么最后一个线程也负责a个元素，否则，最后一个线程负责b个元素
    m_backSortLen = int(m_nums.size()) - (threadNum - 1) * quotient;

    m_pArgs.resize(threadNum);
    m_threads.resize(threadNum);
    m_barrier.setBarrierCount(threadNum + 1); // 留一个给主线程
}

int MultiThreadGroup::getThreadNum() const {
    return m_frontThreadNum + 1; // 前面n-1个线程加上最后一个线程
}

int MultiThreadGroup::getPerGroupLen() const {
    return m_frontSortLen;
}

void MultiThreadGroup::group() {
    int start;
    int end;
    Args *args;

    int threadNum = getThreadNum();
    for (int i = 0; i < threadNum; ++i) {
        start = i * m_frontSortLen;
        if (i < m_frontThreadNum) {
            // 前面n-1个线程
            end = start + m_frontSortLen - 1;
        } else if (i == m_frontThreadNum) {
            // 最后一个线程
            end = start + m_backSortLen - 1;
        }

        /**
         * 此处的args，不能是栈上的局部对象，即不能用Args args(xxx)，因为局部对象超出代码块之后就释放，
         * 而结构体中有vector的引用，一释放相当于整个vector都消失了，所以程序必定会crash。
         * 因此，只能考虑new在堆上。
         * 而且不能用完马上就释放，要等到最后才能释放。所以，不能用智能指针，因为RAII超出作用域就消失。
         */
        args = new Args(m_nums, start, end);
        m_pArgs[i] = args;
        m_threads[i] = thread(std::bind(&MultiThreadGroup::threadFunc, this, args));
    }
}

int MultiThreadGroup::partition(vector<int> &nums, int low, int high) {
    if (low < 0) {
        return -1;
    }
    if (high >= int(nums.size())) {
        return -1;
    }

    int pivot = nums[low];

    while (low < high) {
        while (low < high && nums[high] >= pivot) {
            --high;
        }
        nums[low] = nums[high];

        while (low < high && nums[low] <= pivot) {
            ++low;
        }
        nums[high] = nums[low];
    }

    nums[low] = pivot;

    return low;
}

void MultiThreadGroup::quickSort(vector<int> &nums, int low, int high) {
    if (low < 0) {
        return;
    }
    if (high >= int(nums.size())) {
        return;
    }
    if (low >= high) {
        return;
    }

    int pivotLoc = partition(nums, low, high);
    if (-1 == pivotLoc) {
        return;
    }

    quickSort(nums, low, pivotLoc);
    quickSort(nums, pivotLoc + 1, high);
}

void MultiThreadGroup::threadFunc(Args *args) {
    if (nullptr == args) {
        return;
    }

    {
        mutex mtx;
        unique_lock<mutex> uniqueLock(mtx);
        quickSort(args->nums, args->start, args->end);
    }

    m_barrier.waitAndNotify();
}

MultiThreadGroup::~MultiThreadGroup() {
    for (Args *ptr : m_pArgs) {
        delete ptr;
        ptr = nullptr;
    }
}