#pragma once

#include "huffman_tree.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <exception>

using std::map;
using std::vector;
using std::fstream;
using std::ios;
using std::to_string;
using std::noskipws;

class Compressor {
public:
    explicit Compressor(string fileName) : m_fileName(std::move(fileName)),
                                           m_compressName(m_fileName + ".huffman"),
                                           m_configName(m_fileName + ".config") {
    }

    void compress() {
        // 统计字符及其频数
        map<string, int> &&char2count = statChar2Count();

        // 生成哈夫曼编码
        map <string, string> &&char2code = getHuffmanCode(char2count);

        // 当前存储了多少位
        int bitNum = 0;

        // 将字符按哈夫曼编码进行逐字节存储
        char byte = 0;

        // 读入待压缩文件
        fstream originalFile(m_fileName, ios::in | ios::binary);
        if (!originalFile.is_open()) {
            throw std::exception();
        }
        originalFile >> noskipws;
        originalFile.clear(ios::eofbit);
        originalFile.seekg(0, ios::beg);

        string compressBuffer;
        unsigned char inputChar[2]; // 读入的字符

        while (true) {
            originalFile >> inputChar[0];
            if (originalFile.eof()) {
                break;
            }
            // 判断是中文字符还是英文字符，然后生成相应读入的"字符"
            string curr;
            if (0xB0 <= inputChar[0] && inputChar[0] <= 0xF7) {
                // GB2312中汉字的编码范围为：第一个字节0xB0-0xF7，第二个字节0xA0-0xFE
                curr.push_back(inputChar[0]);
                originalFile >> inputChar[1];
                curr.push_back(inputChar[1]);
            } else if (0 <= inputChar[0] && inputChar[0] <= 255) {
                // ASCII字符
                curr.push_back(inputChar[0]);
            }

            // 根据字符的哈夫曼编码对待输出的字节进行填充
            const string &code = char2code[curr];
            for (char cd : code) {
                // 左移一位
                byte <<= 1;

                // ch当前位
                int currBit = cd - '0';
                byte |= (1 == currBit) ? 1 : 0;
                ++bitNum;

                // 存储了8位，满一个字节，就放到缓冲区中，然后重置
                if (8 == bitNum) {
                    compressBuffer.push_back(byte);
                    m_codes.push_back(byte);
                    byte = 0;
                    bitNum = 0;
                }
            }
        }

        // 文件读完，位数未满8位，写了一个不完整的字节到文件中，就补齐0
        if (0 < bitNum && bitNum < 8) {
            byte <<= (8 - bitNum);
            compressBuffer.push_back(byte);
            m_codes.push_back(byte);
        }

        // 写入压缩文件
        fstream compressFile(m_compressName, ios::out | ios::binary);
        if (!compressFile.is_open()) {
            throw std::exception();
        }
        compressFile >> noskipws;
        compressFile.write(compressBuffer.c_str(), (int) compressBuffer.length());

        // 字符及其频数写入配置文件
        fstream configFile(m_configName, ios::out | ios::binary);
        if (!configFile.is_open()) {
            throw std::exception();
        }
        configFile >> noskipws;

        map<string, int>::const_iterator iter;
        string configBuffer;
        for (iter = char2count.begin(); iter != char2count.end(); ++iter) {
            string letter = std::move(iter->first);
            int count = iter->second;
            configBuffer.append(std::move(letter));
            configBuffer.push_back(':');
            configBuffer.append(std::move(to_string(count)));
            configBuffer.push_back('\n'); // 分隔符
        }
        configFile.write(configBuffer.c_str(), (int) configBuffer.length());

        compressFile.close();
        configFile.close();
        originalFile.close();
    }

    // 返回已经输出的所有哈夫曼编码字符
    vector<char> getCodes() {
        return m_codes;
    }

private:
    string m_fileName; // 原始待压缩文本文件名
    string m_configName; // 配置文件文件名
    string m_compressName; // 压缩文件名
    vector<char> m_codes; // 已经输出的哈夫曼编码，用于测试

    // 统计待压缩文件中字符及其频数
    map<string, int> statChar2Count() {
        fstream originalFile(m_fileName, ios::in | ios::binary);
        if (!originalFile.is_open()) {
            throw std::exception();
        }
        originalFile >> noskipws;

        map<string, int> char2count;
        unsigned char inputChar[2]; // 读入的字符

        while (true) {
            originalFile >> inputChar[0];
            if (originalFile.eof()) {
                break;
            }
            string ch;
            if (0xB0 <= inputChar[0] && inputChar[0] <= 0xF7) {
                // GB2312中汉字的编码范围为：第一个字节0xB0-0xF7，第二个字节0xA0-0xFE
                ch.push_back(inputChar[0]);
                originalFile >> inputChar[1];
                ch.push_back(inputChar[1]);
            } else if (0 <= inputChar[0] && inputChar[0] <= 255) {
                // ASCII字符
                ch.push_back(inputChar[0]);
            }
            ++char2count[ch];
        }

        originalFile.close();
        return char2count;
    }

    // 生成哈夫曼编码
    map <string, string> getHuffmanCode(const map<string, int> &char2count) {
        HuffmanTree huffmanTree(char2count);
        huffmanTree.build();
        map <string, string> &&char2code = huffmanTree.encode();
        return char2code;
    }
};