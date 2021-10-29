#include "multithread_group.h"
#include <random>
#include <ctime>

#define ARRAY_SIZE 1000
#define THREAD_NUM 7

int genRandomNum(int min, int max) {
    static std::default_random_engine e(time(nullptr));
    static std::uniform_int_distribution<int> u(min, max);
    return u(e);
}

void preprocess(vector<int> &nums) {
    // 添加随机数
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        int item = genRandomNum(0, 100000);
        nums.push_back(item);
    }
}

int main() {
    vector<int> nums;
    preprocess(nums); // 准备数组

    int threadNum = THREAD_NUM;
    /**
     * 我们希望在所有线程都碰到栅栏的时候，还不要让所有线程都执行下去，即不要让线程立马退出，
     * 而是让所有线程都碰到栅栏的时候，再让主线程碰一下，之后才让子线程们都退出，然后让主线程去完成归并操作。
     * 因此，init要指定等待的线程个数为threadNum + 1，让每个子线程都执行一次wait，
     * 而主线程的wait在子线程都碰到栅栏之后调用就好。
     */
    Barrier barrier(threadNum + 1);
    vector<thread> threads(threadNum);
    MultiThreadGroup multiThreadGroup(nums, barrier, threads);
    threadNum = multiThreadGroup.getThreadNum(); // 线程不能太多，需要调整线程个数

    multiThreadGroup.group(); // 多个线程分别从小到大排序，实现分组

    // 让主线程碰到栅栏后完成后续工作
    barrier.waitAndNotify();

    for (int i = 0; i < threadNum; ++i) {
        threads[i].join();
    }

    int sortLen = multiThreadGroup.getPerGroupLen();
    for (int i = 1; i <= ARRAY_SIZE; ++i) {
        printf("%d\n", nums[i - 1]);
        if (0 == i % sortLen) {
            printf("==========\n");
        }
    }

    return 0;
}