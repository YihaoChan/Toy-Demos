#pragma once

#include <string>
#include <utility>

using std::string;

struct TreeNode {
    string m_data; // 字符
    int m_weight; // 权重
    TreeNode *m_left; // 左子树
    TreeNode *m_right; // 右子树
    bool m_isLeaf; // 是否为叶结点

    TreeNode() = default;

    ~TreeNode() = default;

    TreeNode(string data, int weight, TreeNode *left, TreeNode *right, bool isLeaf) {
        m_data = std::move(data);
        m_weight = weight;
        m_left = left;
        m_right = right;
        m_isLeaf = isLeaf;
    }

    bool operator<(const TreeNode &rhs) const {
        if (this->m_weight == rhs.m_weight) {
            return this->m_data < rhs.m_data;
        }
        return this->m_weight < rhs.m_weight;
    }

    bool operator>(const TreeNode &rhs) const {
        if (this->m_weight == rhs.m_weight) {
            return this->m_data > rhs.m_data;
        }
        return this->m_weight > rhs.m_weight;
    }
};
