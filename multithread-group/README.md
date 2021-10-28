# 多线程分组

## 1 简介

原本想实现多线程排序的，即：用多个线程对一个很大的数组按不同部分分别排序，最后再归并。但后来发现归并的过程复杂度很高，也没有想到更好的解决办法，所以就只完成到按不同部分分别排序，即实现一个"分组"的过程，每一组都是从小到大有序，也当做练习pthread和thread库的使用。

## 2 运行环境

- 操作系统：Ubuntu 20.04
- make
- C++ 11

```shell
make
./main
make clean
```

## 参考资料

1. [在类中实现多线程-1](https://blog.csdn.net/u014571011/article/details/76283676)
2. [在类中实现多线程-2](https://blog.csdn.net/jmh1996/article/details/72235232)
3. [通过空指针可以正确调用一些类的成员函数？](https://blog.csdn.net/digitalkee/article/details/103192503)
4. [静态方法间接访问非静态方法-1](https://stackoverflow.com/questions/35370901/call-non-static-variable-from-a-static-function)
5. [静态方法间接访问非静态方法-2](https://forum.openframeworks.cc/t/how-to-call-non-static-member-function-from-static-member-function/26656/2)
6. [多线程排序-均摊思想](https://blog.csdn.net/hyb612/article/details/103705632)
7. [pthread_barrier使用](https://blog.csdn.net/jackailson/article/details/51052609)
8. [C++11生成随机数](https://blog.csdn.net/tsbyj/article/details/46994851)
9. [-pthread和-lpthread](https://blog.csdn.net/jakejohn/article/details/79825086)

