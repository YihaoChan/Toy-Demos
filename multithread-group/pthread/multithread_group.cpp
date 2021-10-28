#include "multithread_group.h"

pthread_barrier_t barrier;
pthread_mutex_t mutex;

MultiThreadGroup::MultiThreadGroup(vector<int> &nums, vector<pthread_t> &threads) :
        m_nums(nums),
        m_frontSortLen(0),
        m_frontThreadNum(0),
        m_backSortLen(0),
        m_threads(threads) {
    // 将线程个数调整至合适，并为每个线程分配负责的元素个数
    dispatchNumsForThreads(int(threads.size()));
    pthread_mutex_init(&mutex, nullptr);
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
        pthread_create(&m_threads[i], nullptr, threadFunc, args);
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

/**
 * C++类的成员函数的函数指针不能直接做为参数传到pthread_create，
 * 主要是因为是C++成员函数在编译时末尾会被编译器加上可以接收对象地址的this指针参数，
 * 而pthread_create只能接受静态函数的函数指针，而带有this的函数指针是对象的，和静态这一点冲突。
 * 只能使用静态函数将带来一个问题，静态成员函数无法访问非静态的成员，这将带来许多不便，
 * 总不能将要用到的成员变量也改成静态的，这是不推荐的。
 * 因此，可以采用类似跳板的方式来解决：将函数指针定义为static，放到pthread_create满足作为参数的条件，而真正执行功能的部分放在这个"包装"当中。
 */
void *MultiThreadGroup::threadFunc(void *args) {
    if (nullptr == args) {
        return nullptr;
    }

    /**
     * 一个静态方法根本不可能【直接】访问非静态方法，即普通的sortPerThread()，需要用间接的形式
     *
     * e.g. (1)
     * The only way to access non-static elements in a static member function
     * is to tell the function [which object] it has to use for getting access non-static function.
     * class MyClass {
     *     bool eventActive = false;
     *     static bool JoinCommand(MyClass *object) {
     *         // No error, since we are referencing through an object.
     *         if (object->eventActive) {
     *             // do something here...
     *         }
     *     }
     *  };
     *
     *  e.g. (2)
     *  Let the static function give you a [pointer] to the class instance:
     *  class OfApp {
     *  void update(); //non Static Function
     *  static void staticFunction(void * userData) {
     *      auto app = (OfApp*)userData;
     *      app->update();
     *  }
     *
     *  简而言之，就是在静态方法中，用类的【对象指针】去访问非静态成员方法
     *  所以，下面的调用还可以这样写：
     *  MultiThreadGroup *m = nullptr;
     *  m->sortPerThread();
     */
    static_cast<MultiThreadGroup *>(nullptr)->sortPerThread((Args *) args);
    return nullptr;
}

void MultiThreadGroup::sortPerThread(Args *args) {
    if (nullptr == args) {
        return;
    }

    pthread_mutex_lock(&mutex);
    quickSort(args->nums, args->start, args->end);
    pthread_mutex_unlock(&mutex);

    // 报道函数：当一个线程到达栅栏的时候，就报道碰到栅栏
    pthread_barrier_wait(&barrier);
}

MultiThreadGroup::~MultiThreadGroup() {
    for (Args *ptr : m_pArgs) {
        delete ptr;
        ptr = nullptr;
    }
    pthread_mutex_destroy(&mutex);
}