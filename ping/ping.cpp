#include "ping.h"

int sockfd;
pid_t pid;
char *hostname; // 域名
struct sockaddr_in dst_addr; // 目标主机的ip
static int sent_num = 1; // 发送的报文数量
static int recv_num = 1; // 接收到的报文数量
static struct sockaddr_in reply_addr; // 发送IP响应的实际主机的ip
static struct sigaction act_alarm; // 捕获SIGALRM信号
static struct sigaction act_int; // 捕获SIGINT信号

// 发送ping命令实际执行的ICMP请求包
void send_icmp_request() {
    char send_buffer[BUFFER_SIZE];
    struct icmp_header *icmp_hdr = (struct icmp_header *) send_buffer;
    icmp_hdr->icmp_type = ICMP_ECHO_REQUEST; // ICMP请求
    icmp_hdr->icmp_code = 0;
    icmp_hdr->icmp_id = pid;
    icmp_hdr->icmp_seq = sent_num++;
    icmp_hdr->icmp_checksum = 0; // 校验和置零
    memset(icmp_hdr->icmp_data, 0, sizeof(icmp_hdr->icmp_data));
    gettimeofday((struct timeval *) icmp_hdr->icmp_timestamp, nullptr);

    /**
     * 发送数据时：
     * 1、把校验和字段设置为0；
     * 2、把需要校验的数据看成以16位为单位的数字组成，依次进行二进制求和，然后取反码；
     * 3、把得到的结果存入校验和字段中。
     */
    icmp_hdr->icmp_checksum = compute_checksum((u8 *) icmp_hdr, ICMP_LEN);

    // 原始套接字，用sendto、recvfrom发送和接收
    sendto(sockfd, icmp_hdr, ICMP_LEN, 0, (struct sockaddr *) &dst_addr, sizeof(dst_addr));
}

// 解析发回来的IP响应包
int parse_ip_reply(char (&recv_buffer)[BUFFER_SIZE]) {
    struct ip_header *ip_hdr = (struct ip_header *) recv_buffer;
    // 首部长度：占4位，可表示的最大十进制数值是15。注意：这个字段所表示数的单位是32位字(1个32位字长是4字节)。
    // 因此，当IP的首部长度为1111时(即十进制的15)，首部长度就达到60字节。
    int ip_header_len = ip_hdr->ip_hl << 2;

    // ICMP数据报是IP报文的数据部分
    struct icmp_header *icmp_hdr = (struct icmp_header *) (recv_buffer + ip_header_len);
    // ICMP包总长度 = IP包数据部分长度
    u16 ip_data_len = ntohs(ip_hdr->ip_totallen) - ip_header_len; // 网络字节序(大端法)转主机字节序(小端法)
    u16 icmp_len = ip_data_len;

    // ICMP响应
    if (icmp_hdr->icmp_type != ICMP_ECHO_REPLY) {
        return -1;
    }
    // 标识符
    if (icmp_hdr->icmp_id != pid) {
        return -1;
    }
    /**
     * 接收数据时：
     * 1、把首部看成以16位为单位的数字组成，对整个报文(ICMP报头+ICMP数据)依次进行二进制求和，包括校验和字段，最后取反码；
     * 2、检查计算出的结果是否为0；
     * 3、如果等于0，说明校验和正确。否则，校验和错误，协议栈要抛弃这个数据包。
     */
    u16 checksum = compute_checksum((u8 *) icmp_hdr, icmp_len);
    if (checksum != 0) {
        return -1;
    }

    // 发送响应包时的时间
    struct timeval *send_reply_time = (struct timeval *) icmp_hdr->icmp_timestamp;
    // 收到响应包时的时间
    struct timeval recv_reply_time;
    gettimeofday(&recv_reply_time, nullptr);

    // 往返时间
    double rtt = ((&recv_reply_time)->tv_sec - send_reply_time->tv_sec) * 1000 +
                 ((&recv_reply_time)->tv_usec - send_reply_time->tv_usec) / 1000.0;

    printf("%d bytes from %s: icmp_seq=%u ttl=%d rtt=%.1f ms\n",
           icmp_len,
           inet_ntoa(reply_addr.sin_addr),
           icmp_hdr->icmp_seq,
           ip_hdr->ip_ttl,
           rtt);

    return 0;
}

// 接收所有的ping命令响应包
void recv_ping_reply() {
    /**
     * 由于SIGALRM定时产生且被捕获，因此会一直发送ICMP请求。收到ICMP响应后就会进入解析响应包的函数，直到到达显示的次数。
     * 只要recv_num还没到达最大数量，就继续循环。
     * 因此，大致流程为：
     * SIGALRM信号一直在产生，使得ICMP请求被发送。
     * 而recv_ping_reply函数在main函数中被调用，一被调用就进入while循环，直到处理次数到达最大次数才返回到main中结束进程。
     */
    socklen_t len = sizeof(reply_addr);
    char recv_buffer[BUFFER_SIZE];
    while (recv_num <= MAX_ECHO_TIMES) {
        // 有ICMP请求被发送过去过，recv才有数据，否则阻塞，直到响应包到达
        int recv_len = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0,
                                (struct sockaddr *) &reply_addr, &len);
        if (recv_len < 0) {
            // 有可能是接收错误，也有可能信号中断，error被设置成EINTR
            continue;
        } else if (0 == recv_len) {
            continue;
        }

        if (parse_ip_reply(recv_buffer) != 0) {
            // IP响应错误，抛弃这个数据包
            continue;
        }

        ++recv_num;
    }

    sleep(1);
    end();
}

// 计算校验和
u16 compute_checksum(u8 *buffer, int len) {
    u32 sum = 0;
    u16 *buf;
    buf = (u16 *) buffer; // 按16字节为单位进行读取、求和

    while (len > 1) {
        sum += *buf;
        ++buf;
        len -= 2; // 按16字节为单位进行读取、求和
    }
    // 数据部分为变长非固定长度，所以不一定为偶数
    if (1 == len) {
        sum += *(u8 *) buf;
    }

    // 最高位有进位，则把最高位加到最低位
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);

    // 取反
    return (u16) ~sum;
}

// 统计ping命令的检测结果
void get_statistics(const int n_sent, const int n_recv) {
    printf("--- %s ping statistics ---\n", hostname);
    printf("%d packets transmitted, %d received, %0.0f%% packet loss\n",
           n_sent, n_recv, 1.0 * (n_sent - n_recv) / n_sent * 100);
}

// 结束
void end() {
    get_statistics(sent_num, recv_num);
    close(sockfd);
}

// 注册信号处理函数
void set_sighandler() {
    act_alarm.sa_handler = alarm_handler;
    if (-1 == sigaction(SIGALRM, &act_alarm, nullptr)) {
        printf("SIGALRM handler setting fails.\n");
        exit(1);
    }

    act_int.sa_handler = int_handler;
    if (-1 == sigaction(SIGINT, &act_int, nullptr)) {
        printf("SIGINT handler setting fails.\n");
        exit(1);
    }
}

// SIGINT
void int_handler(int sig) {
    printf("\n");
    // 信号被捕获时，输出统计信息，然后关闭
    end();
    exit(1);
}

// SIGALRM
void alarm_handler(int signo) {
    // 定时器设定时间，每隔这个设定的时间就发送一个SIGALRM信号，然后对其捕获，发送ICMP数据包，实现一直打印新的响应报文的效果
    send_icmp_request();
}