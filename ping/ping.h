#pragma once

#include <stdio.h>
#include <cstdlib>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <cerrno>
#include <arpa/inet.h>
#include <csignal>
#include <netinet/in.h>

#ifndef _LITTLE_ENDIAN_BITFIELD
#define _LITTLE_ENDIAN_BITFIELD
#endif

// 数据类型别名
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

// IP报文头部
struct ip_header {
#if defined _LITTLE_ENDIAN_BITFIELD
    u8 ip_hl: 4; // 头部长度，位域占4位
    u8 ip_ver: 4; // 版本号，位域占4位
#elif defined _BIG_ENFIAN_BITFELD
    u8 ip_ver: 4;
    u8 ip_hl: 4;
#endif
    u8 ip_tos; // 服务类型
    u16 ip_totallen; // 总长度
    u16 ip_id; // 标志位
    u16 ip_flag: 3; // 标识符，位域占3位
    u16 ip_frag_off: 13; // 片偏移，位域占13位
    u8 ip_ttl; // 生存时间TTL
    u8 ip_protocol; // 协议类型
    u16 ip_checksum; // 头部校验和
    u32 ip_src_addr; // 源IP地址
    u32 ip_dst_addr; // 目标IP地址
};

// ICMP报文头部
struct icmp_header {
    u8 icmp_type; // 类型
    u8 icmp_code; // 代码
    u16 icmp_checksum; // 校验和

    union {
        // 查询请求和应答消息格式
        struct {
            u16 id;
            u16 seq;
        } echo;

        u32 gateway;

        struct {
            u16 unused;
            u16 mtu; // pmtu(路径MTU)发现用来确定到达目的地的路径中最大传输单元(MTU)的大小
        } frag;
    } icmp_un;

    u8 icmp_data[0]; // 数据

    /* This is not the std header. */
    u32 icmp_timestamp[2]; // 时间戳

#define icmp_id icmp_un.echo.id
#define icmp_seq icmp_un.echo.seq
};

#define ICMP_LEN 56 // ICMP报文默认大小

#define ICMP_ECHO_REPLY 0 // ICMP响应
#define ICMP_ECHO_REQUEST 8 // ICMP请求

#define BUFFER_SIZE 1024 // 缓冲区大小

#define MAX_ECHO_TIMES 4 // 在屏幕上打印几条ping信息

/**
 * #ifndef XXX_H
 * #define XXX_H
 * 只能保证在一个源文件中不会重复多次引用。
 * 而现在是在两个cpp文件中都引用相同的全局变量，而且这两个文件要链接成一个可执行文件，所以会有两处定义。
 * 最佳做法是头文件不能有变量的定义，可以有声明。因此，用extern关键字，且在使用时要在某个cpp文件中定义一下使用到的全局变量。
 */
extern int sockfd;
extern pid_t pid;
extern struct sockaddr_in dst_addr; // 目标主机的ip
extern struct itimerval itv; // 倒计时
extern char *hostname; // 域名

// 发送ping命令实际执行的ICMP请求包
void send_icmp_request();

// 接收所有的ping命令响应
void recv_ping_reply();

// 解析发回来的IP响应包
int parse_ip_reply(char (&recv_buffer)[BUFFER_SIZE]);

// 计算校验和
u16 compute_checksum(u8 *buffer, int len);

// 统计ping命令的检测结果
void get_statistics(const int n_sent, const int n_recv);

// 结束
void end();

// 注册信号处理函数
void set_sighandler();

// SIGINT
void int_handler(int sig);

// SIGALRM
void alarm_handler(int signo);