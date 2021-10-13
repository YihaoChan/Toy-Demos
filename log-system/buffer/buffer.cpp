#include "buffer.h"

Buffer::Buffer(int bufferSize) : m_bufferSize(bufferSize),
                                 m_buffer(bufferSize),
                                 m_readableIndex(0),
                                 m_writeableIndex(0) {}

Buffer::~Buffer() {
    m_buffer.clear();
    m_buffer.shrink_to_fit(); // 释放空间
}

// 还要读多少字节才碰到已经写到的位置
int Buffer::getReadableSize() {
    return m_writeableIndex - m_readableIndex;
}

// 还要写多少字节才碰到缓冲区的最大大小
int Buffer::getWritableSize() {
    return m_bufferSize - m_writeableIndex;
}

// 添加数据到buffer中
void Buffer::append(const char *data, int len) {
    // 要把一个序列拷贝到一个容器中，通常用std::copy算法
    std::copy(data, data + len, getWritePtr());
    moveWriteIndex(len);
    return;
}

// buffer中数据的起始指针
char *Buffer::begin() {
    return &(*(m_buffer.begin()));
}

// 获得读位置的指针
char *Buffer::getReadPtr() {
    return begin() + m_readableIndex;
}

// 获得写位置的指针
char *Buffer::getWritePtr() {
    return begin() + m_writeableIndex;
}

// 移动已经写到buffer中的位置下标
void Buffer::moveWriteIndex(int len) {
    m_writeableIndex += len;
    return;
}

// 重置buffer
void Buffer::resetBuffer() {
    m_readableIndex = 0;
    m_writeableIndex = 0;
    m_buffer.clear();
    return;
}