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
    // ��������
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        int item = genRandomNum(0, 100000);
        nums.push_back(item);
    }
}

int main() {
    vector<int> nums;
    preprocess(nums); // ׼������

    int threadNum = THREAD_NUM;
    /**
     * ����ϣ���������̶߳�����դ����ʱ�򣬻���Ҫ�������̶߳�ִ����ȥ������Ҫ���߳������˳���
     * �����������̶߳�����դ����ʱ���������߳���һ�£�֮��������߳��Ƕ��˳���Ȼ�������߳�ȥ��ɹ鲢������
     * ��ˣ�initҪָ���ȴ����̸߳���ΪthreadNum + 1����ÿ�����̶߳�ִ��һ��wait��
     * �����̵߳�wait�����̶߳�����դ��֮����þͺá�
     */
    Barrier barrier(threadNum + 1);
    vector<thread> threads(threadNum);
    MultiThreadGroup multiThreadGroup(nums, barrier, threads);
    threadNum = multiThreadGroup.getThreadNum(); // �̲߳���̫�࣬��Ҫ�����̸߳���

    multiThreadGroup.group(); // ����̷ֱ߳��С��������ʵ�ַ���

    // �����߳�����դ������ɺ�������
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