#pragma once

#include <vector>
#include <map>
#include "tree_node.h"

using std::vector;
using std::map;
using std::swap;

class MinHeap {
public:
    MinHeap() : m_size(0) {}

    // 往堆中插入元素
    void push(TreeNode *treeNode) {
        if (nullptr == treeNode) {
            return;
        }
        m_arr.push_back(treeNode);
        ++m_size;
    }

    // 建堆
    void buildHeap() {
        // 从最后一个非叶子结点开始下滤
        for (int i = (m_size / 2) - 1; i >= 0; --i) {
            percolateDown(m_arr[i], i, m_size);
        }
    }

    // 查看堆顶元素
    TreeNode *top() const {
        if (m_size > 0) {
            return m_arr[0];
        }
        return nullptr;
    }

    // 弹出堆顶结点
    void pop() {
        if (m_size <= 0) {
            return;
        }
        // 最后一个元素和堆顶元素交换，然后弹出原堆顶元素
        swap(m_arr[0], m_arr[m_size - 1]);
        m_arr.pop_back();
        // 从堆顶新元素处开始下滤
        TreeNode *item = m_arr[0];
        --m_size;
        percolateDown(item, 0, m_size);
    }

    // 堆中元素个数
    int size() const {
        return m_size;
    }

    ~MinHeap() {
        for (int i = 0; i < m_size; ++i) {
            TreeNode *treeNode = m_arr[i];
            delete treeNode;
            treeNode = nullptr;
        }
        m_arr.clear();
        m_arr.shrink_to_fit();
    }

private:
    vector<TreeNode *> m_arr; // 存放堆中元素的数组
    int m_size; // 堆中共有几个元素

    // 下滤
    void percolateDown(TreeNode *item, int start, int end) {
        int parent, child;
        for (parent = start; parent * 2 + 1 < end; parent = child) {
            child = parent * 2 + 1; // 左子结点下标
            if ((child != end - 1) && (*(m_arr[child + 1]) < *(m_arr[child]))) {
                ++child; // 右子结点较小
            }
            if (*item > *(m_arr[child])) {
                m_arr[parent] = m_arr[child];
            } else {
                break;
            }
        }
        // 下滤完毕，填入要调整的元素
        m_arr[parent] = item;
    }
};
