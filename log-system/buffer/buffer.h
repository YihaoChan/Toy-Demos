#ifndef BUFFER_H_
#define BUFFER_H_

#include <vector>

using std::vector;

class Buffer {
public:
    Buffer(int bufferSize);

    ~Buffer();

    int getReadableSize(); // 还要读多少字节才碰到已经写到的位置

    int getWritableSize(); // 还要写多少字节才碰到缓冲区的最大大小

    void append(const char *data, int len); // 添加数据到buffer中

    char *begin(); // buffer中数据的起始指针

    char *getReadPtr(); // 获得读位置的指针

    char *getWritePtr(); // 获得写位置的指针

    void resetBuffer(); // 重置buffer

private:
    void moveWriteIndex(int len); // 移动已经写到buffer中的位置下标

    vector<char> m_buffer; // 缓冲区，用数组实现
    int m_bufferSize; // 缓冲区大小
    int m_readableIndex; // 已经读到的位置
    int m_writeableIndex; // 已经写到的位置
};

#endif
