# 简易异步日志系统

# 1 简单想法

假如自己写一个日志程序，最原始且简单的想法应该是：首先将要写入日志文件的内容转化为字符串，然后调用write系统调用将其写入文件。这种实现方法的确就是最原始的日志方法，但无疑是十分低效的，那么如何让日志能够高效起来？需要实现异步日志。

# 2 实现目标

简单想法中，日志系统低效的原因为：

1. 每条日志内容都会调用write系统调用，写一条日志就调用一次它，势必系统开销会很大；
2. 当在关键的地方调用write时，会不会导致关键部分的代码不能及时被执行？

上述两个导致原始日志系统低效的主要原因的解决方案分别为:

1. 既然每条日志都调用一次write会导致系统开销变大，那么就设计一个buffer，将日志内容保存在buffer中，待buffer满之后再一次性调用write将其写入磁盘文件即可；
2. 由于write可能会阻塞在当前位置，导致紧随其后的关键代码可能不能马上执行，那么就单独开个线程专门来执行write调用就可以了。

根据两个初步的解决方案，可以总结出异步日志的基本流程应该为：

**前端线程(调用日志系统的程序)负责将日志写入buffer中，当buffer写满之后，将buffer转交给后端线程(负责专门调用write来写日志的线程)，让后端线程适时进行写盘操作，也就是一个典型的多生产者-单消费者模式**。

多生产者-单消费者，又名有界缓冲区(bounded-buffer)问题：(1)当缓冲区已满，而此时生产者还想向缓冲区中放入一个新的数据项时，则让生产者睡眠，待消费者从缓冲区中取出一个或多个数据项时再唤醒生产者；(2)
当缓冲区满已空，而此时消费者还想从缓冲区中取出一个新的数据项时，则让消费者睡眠，待生产者向缓冲区中放入一个或多个数据项时再唤醒消费者。同时，多个生产者需要多添加一个条件变量和互斥变量，用来保证多个生产者之间对消息队列的互斥访问。

因为日志系统全局只有这一份，所以采用单例模式创建，同时借用C++11特性，创建**局部静态变量**以保证线程安全。

# 3 滚动日志

1. 当单个文件大小超过文件最大容量时，就创建一个新的文件；
2. 如果创建的日志文件数量多于这个数值，就压缩最旧的一个文件以节省存储空间。这样设计的原因是大部分系统管理员都只需要查阅一定数量内的日志文件，其余的日志文件压缩起来，需要的时候可以解压查阅，这是一个很好的折中方案。

# 4 运行

- 所使用操作系统：Ubuntu 20.04
- cmake
- make

```shell
cd ./build
cmake .
make
cd ..
./main
```

# 参考资料

1. [一个高效的异步日志](https://www.cnblogs.com/firstdream/p/6807706.html)

2. [使用C++11实现线程安全的单例模式](https://blog.csdn.net/zyhse/article/details/105336468)

3. [C++11多线程std::thread的简单使用](https://www.cnblogs.com/lifan3a/articles/7538472.html)

4. [C++多线程并发---线程同步之条件变量](https://blog.csdn.net/m0_37621078/article/details/89766449)