#pragma once

#include <fstream>
#include <utility>
#include <vector>
#include <map>
#include <string.h>
#include "huffman_tree.hpp"

using std::map;
using std::vector;
using std::fstream;
using std::ios;
using std::to_string;
using std::noskipws;
using std::stoi;

class Decompressor {
public:
    explicit Decompressor(string compressName) : m_compressName(std::move(compressName)) {
        string temp = m_compressName;
        string::size_type pos = temp.rfind('.') + 1;
        m_configName = temp.replace(pos, strlen("huffman"), "config");
        temp = m_compressName;
        m_decompressName = temp.replace(pos, strlen("huffman"), "decompress");
    }

    void decompress() {
        // 统计字符及其频数
        map<string, int> &&char2count = statChar2Count();

        // 哈夫曼树根结点
        TreeNode *root = getHuffmanRoot(char2count);
        TreeNode *node = root;

        // 读入压缩文件
        fstream compressFile(m_compressName, ios::in | ios::binary);
        if (!compressFile.is_open()) {
            throw std::exception();
        }
        compressFile >> noskipws;

        char ch;
        string decompressBuffer;
        while (compressFile.get(ch)) {
            // 一个字节读8位
            int bitNum = 8;

            while (bitNum > 0) {
                if (node->m_isLeaf) {
                    // 到了叶结点，就把哈夫曼编码对应字符取出来，输出到文件
                    string realChar = node->m_data;
                    decompressBuffer.append(realChar);

                    // 重置当前结点为根结点
                    node = root;

                    // 已统计到的字符对应出现次数-1
                    --char2count[realChar];
                    if (char2count[realChar] == 0) {
                        char2count.erase(realChar);
                        // 当所有字符的编码都已经转成字符时，直接终止
                        if (char2count.empty()) {
                            goto END;
                        }
                    }
                }

                // 把最高位逻辑右移到最低位，如10001010变成00000001
                char highest = (unsigned char) (ch) >> 7;

                // 读取每个字节并判断每一位，如果为1，则哈夫曼树往右走，如果为0，则哈夫曼树往左走，直到叶结点时返回实际存储字符
                if (highest == 1) {
                    node = node->m_right;
                } else if (highest == 0) {
                    node = node->m_left;
                }

                --bitNum;
                // 一直制造出每一轮的最高位
                ch <<= 1;
            }
        }
        // 所有字符的编码都已经转成字符，直接跳到此处
        END:
        fstream decompressFile(m_decompressName, ios::out | ios::binary);
        if (!decompressFile.is_open()) {
            throw std::exception();
        }
        decompressFile >> noskipws;
        decompressFile.write(decompressBuffer.c_str(), (int) decompressBuffer.length());

        compressFile.close();
        decompressFile.close();
    }

    ~Decompressor() {
        if (m_huffmanTree != nullptr) {
            delete m_huffmanTree;
            m_huffmanTree = nullptr;
        }
    }

private:
    string m_configName; // 压缩文件的配置文件名
    string m_compressName; // 压缩文件的文件名
    string m_decompressName; // 解压文件的文件名
    HuffmanTree *m_huffmanTree; // 哈夫曼树对象

    // 统计配置文件中字符及其频数
    map<string, int> statChar2Count() {
        map<string, int> char2count;

        fstream configFile(m_configName, ios::in | ios::binary);
        if (!configFile.is_open()) {
            throw std::exception();
        }
        configFile >> noskipws;

        while (!configFile.eof()) {
            string line;
            getline(configFile, line);

            if (line.length() == 0) {
                // 正常的\r是能完整读到的，如果这一行是\n，那么getline读到的长度就是0
                // 有可能是正常的\n:xxx或最后一行的\n，先加上下一行内容，就可以根据长度判断是正常的\n还是最后一行的\n
                string lineFeed = "\n";
                getline(configFile, line);
                lineFeed.append(std::move(line));
                line = std::move(lineFeed);
            }

            // 最后一行只有换行符，本来getline为0，加了1后变成1。因此，长度不为1的就是正常的\n:xxx。
            if (line.length() == 1) {
                continue;
            }

            if (line.length() != 1) {
                string ch;
                int count;
                // 判断是中文字符还是英文字符，然后生成相应读入的"字符"
                if (0xB0 <= (unsigned char) line[0] && (unsigned char) line[0] <= 0xF7) {
                    // 格式：[gbk][gbk]:[num]
                    string chinese;
                    chinese.push_back(line[0]);
                    chinese.push_back(line[1]);
                    int cnt = stoi(std::move(line.substr(3, line.length() - 3)));
                    ch = std::move(chinese);
                    count = cnt;
                } else if (0 <= (unsigned char) line[0] && (unsigned char) line[0] <= 255) {
                    // 格式：[ascii]:[num]
                    string english;
                    english.push_back(line[0]);
                    int cnt = stoi(std::move(line.substr(2, line.length() - 2)));
                    ch = std::move(english);
                    count = cnt;
                }
                char2count[ch] = count;
            }
        }
        configFile.close();

        return char2count;
    }

    // 获取哈夫曼树根结点
    TreeNode *getHuffmanRoot(const map<string, int> &char2count) {
        // 不能在这里创建局部哈夫曼树对象后把root返回，因为局部对象在函数体结束后就消亡，导致root也失效
        m_huffmanTree = new HuffmanTree(char2count);
        m_huffmanTree->build();
        TreeNode *root = m_huffmanTree->getRoot();
        return root;
    }
};
