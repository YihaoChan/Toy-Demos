#pragma once

#include "min_heap.hpp"
#include "tree_node.h"
#include <string>

using std::string;

class HuffmanTree {
public:
    // 在写一个类的时候，如果一个构造器是接收单个参数的，那么最好要加上explicit。
    // 如果不加的话，该构造函数可能有类型转换的情形，造成混淆。
    explicit HuffmanTree(const map<string, int> &data2weight) : m_root(nullptr) {
        map<string, int>::const_iterator iter;
        for (iter = data2weight.begin(); iter != data2weight.end(); ++iter) {
            string data = std::move(iter->first);
            int weight = iter->second;
            // 初始时每个结点都是一棵哈夫曼树，组成一个森林，且都是叶子结点，代表数据域存放真实字符及其权值(类似于前缀树)
            TreeNode *treeNode = new TreeNode(data, weight, nullptr, nullptr, true);
            // 往最小堆里插入元素
            m_minHeap.push(treeNode);
        }
        // 根据结点建立最小堆
        m_minHeap.buildHeap();
    }

    HuffmanTree() = default;

    // 构建哈夫曼树
    void build() {
        if (0 == m_minHeap.size()) {
            return;
        }
        while (m_minHeap.size() > 1) {
            // 左子结点
            TreeNode *leftChild = m_minHeap.top();
            m_minHeap.pop();
            // 右子结点
            TreeNode *rightChild = m_minHeap.top();
            m_minHeap.pop();
            // 新结点
            TreeNode *newNode = new TreeNode("",
                                             leftChild->m_weight + rightChild->m_weight,
                                             leftChild,
                                             rightChild,
                                             false);
            // 往最小堆中插入
            m_minHeap.push(newNode);
            m_minHeap.buildHeap();
        }
        m_root = m_minHeap.top();
    }

    // 返回哈夫曼树根结点
    TreeNode *getRoot() const {
        return m_root;
    }

    // 生成每个字符的哈夫曼编码
    map<string, string> encode() {
        map<string, string> char2code;
        string track;
        backtrack(m_root, track, char2code);
        return char2code;
    }

    ~HuffmanTree() = default;

private:
    MinHeap m_minHeap; // 最小堆，用于top和pop两个最小权值的结点
    TreeNode *m_root; // 哈夫曼树根结点

    // 回溯
    void backtrack(TreeNode *root, string &track, map<string, string> &char2code) {
        if (nullptr == m_root) {
            return;
        }
        if (root->m_isLeaf) {
            char2code[root->m_data] = track; // 字符 -> 哈夫曼编码
            return;
        }

        track.push_back('0');
        backtrack(root->m_left, track, char2code);
        track.pop_back();

        track.push_back('1');
        backtrack(root->m_right, track, char2code);
        track.pop_back();
    }
};
