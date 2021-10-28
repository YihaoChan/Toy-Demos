# 多线程分组

## 1 简介

原本想实现多线程排序，即：用多个线程对一个很大的数组按不同部分分别排序，最后再归并。但后来发现归并的过程复杂度很高，也没有找到/想到更好的解决办法，所以就只完成到按不同部分分别排序，即实现一个"分组"的过程，每一组都是从小到大有序，也当做练习pthread和thread库的使用。

## 2 屏障机制

1. `pthread_barrier_xxx`只做且只能做一件事，就是充当屏障(栏杆)，形象地说就是把先后到达的多个线程挡在同一栏杆前，直到所有线程到齐，然后撤下栏杆同时放行；
2. `wait`函数由每个线程主动调用，它告诉栏杆："我到起跑线前了"。`wait`执行末尾栏杆会检查是否所有人都到栏杆前了：如果是，栏杆就放下，所有线程继续执行下一句代码；如果不是，则所有已到`wait`的线程停在该函数不动，剩下**没执行到wait的线程**继续执行；
3. `pthread_barrier_t`本质上就是一个计数锁。

这种"栏杆"机制最大的特点就是：最后一个执行`wait`的动作最为重要，就像赛跑时的起跑枪一样，它来之前所有人都必须等着。所以实际使用中，`pthread_barrier_xxx`常常用来让所有线程等待"起跑枪"响起后再一起行动。比如我们可以用`pthread_create`生成100个线程，每个子线程在被create出的瞬间就会自顾自地立刻进入回调函数运行，我们希望它们在回调函数中申请完线程空间、初始化后**停下来，一起等待主进程释放一个"开始"信号，然后所有线程再开始执行后续其他的业务逻辑代码。**

为了解决上述场景问题，我们可以在`init`时指定`n+1`个等待，其中n是线程数。而在每个线程执行函数中调用`wait`。这样，100个`pthread_create`结束后，所有线程都停下来等待最后一个`wait`函数被调用。这个`wait`由主线程决定，在任意时刻都可以调用，最后这个`wait`就是鸣响的起跑枪。

多线程排序的思路就是先让每个线程负责各自的部分，然后停下来，由主线程调用最后一个`wait`之后执行归并。

## 3 运行环境

- 操作系统：Ubuntu 20.04
- make
- C++ 11

```shell
make
./main
make clean
```

## 参考资料

1. [pthread_barrier使用](https://blog.csdn.net/jackailson/article/details/51052609)
2. [多线程排序-均摊思想](https://blog.csdn.net/hyb612/article/details/103705632)
3. [实现一个Barrier](https://www.cnblogs.com/ZXYloveFR/p/11300172.html)
4. [在类中实现多线程-1](https://blog.csdn.net/u014571011/article/details/76283676)
5. [在类中实现多线程-2](https://blog.csdn.net/jmh1996/article/details/72235232)
6. [通过空指针可以正确调用一些类的成员函数？](https://blog.csdn.net/digitalkee/article/details/103192503)
7. [静态方法间接访问非静态方法-1](https://stackoverflow.com/questions/35370901/call-non-static-variable-from-a-static-function)
8. [静态方法间接访问非静态方法-2](https://forum.openframeworks.cc/t/how-to-call-non-static-member-function-from-static-member-function/26656/2)
9. [C++11生成随机数](https://blog.csdn.net/tsbyj/article/details/46994851)
10. [-pthread和-lpthread](https://blog.csdn.net/jakejohn/article/details/79825086)

