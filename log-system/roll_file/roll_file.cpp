#include "roll_file.h"
#include <cassert>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

RollFile::RollFile(int maxFileNum, int maxFileSize) : m_maxFileNum(maxFileNum),
                                                      m_maxFileSize(maxFileSize),
                                                      m_currFileSize(0),
                                                      m_currFileNum(1),
                                                      m_dir("./info/"),
                                                      m_file(""),
                                                      m_fullPath(""),
                                                      m_fd(-1) {
    // 若文件夹不存在则创建
    if (-1 == access(m_dir.c_str(), 0)) {
        mkdir(m_dir.c_str(), 0755);
    }
    // 创建文件
    m_file = m_timestamp.timeUntilDay() + "-{" + std::to_string(m_currFileNum) + "}.txt";
    m_fullPath = m_dir + m_file;
    m_fd = setupFile(m_fullPath.c_str());
    m_fileNameQueue.push(m_fullPath);
}

RollFile::~RollFile() {
    if (m_fd != -1) {
        close(m_fd);
    }
}

// 创建文件
int RollFile::setupFile(const char *fileName) {
    int fd = open(fileName, O_WRONLY | O_APPEND | O_CREAT, S_IWUSR | S_IRUSR | S_IXUSR);
    assert(fd > 0);
    return fd;
}

// 添加日志，并实现滚动日志文件
void RollFile::write2file(char *log, int len) {
    m_currFileSize += len;

    // 如果超过当前文件最大长度，就写到下一个文件中
    if (m_currFileSize >= m_maxFileSize) {
        ++m_currFileNum;
        m_currFileSize = 0;
        m_file = m_timestamp.timeUntilDay() + "-{" + std::to_string(m_currFileNum) + "}.txt";
        m_fullPath = m_dir + m_file;
        m_fd = setupFile(m_fullPath.c_str());
        m_fileNameQueue.push(m_fullPath);
        // 如果文件数量大于最大文件数量，就压缩最旧的那个文件
        if (m_currFileNum > m_maxFileNum) {
            string compressFileName = m_fileNameQueue.front();
            m_fileNameQueue.pop();
            // 当前运行目录
            string cwd = getcwd(NULL, 0);
            // 切换到日志文件夹中压缩
            chdir((const char *) m_dir.c_str());
            string command = "tar -zcf " +
                             compressFileName.substr(m_dir.length()) + ".gz " +
                             compressFileName.substr(m_dir.length());
            system(command.c_str());
            string compressLog = m_timestamp.timeUntilSec() +
                                 " INFO Compress " +
                                 compressFileName.substr(m_dir.length()) +
                                 "\n";
            int ret = write(m_fd, compressLog.c_str(), compressLog.length());
            assert(ret == compressLog.length());
            // 切回运行目录
            chdir((const char *) cwd.c_str());
            // 删除txt文件
            remove(compressFileName.c_str());
        }
    }

    int ret = write(m_fd, log, len);
    assert(ret == len);

    return;
}

