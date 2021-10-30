#include "ping.h"

/*******************************************************************/
/**
 * act_alarm捕获SIGALRM信号，act_int捕获SIGINT信号。
 * 在类里面用包装后的静态函数作为回调函数，静态函数访问非静态的成员函数。
 * 此时，如果在非静态的成员函数中出现非局部变量(如sent_num、dst_addr、pid或hostname)，
 * 那么这些变量也要静态或全局，否则会crash。
 */
static const char *hostname; // 域名
static int sockfd;
static pid_t pid;
static int sent_num = 0; // 发送报文的数量
static int recv_num = 0; // 接收报文的数量
static struct sockaddr_in dst_addr; // 目标主机的ip
static struct sockaddr_in reply_addr; // 发送IP响应的实际主机的ip
static struct sigaction act_alarm; // 捕获SIGALRM信号
static struct sigaction act_int; // 捕获SIGINT信号
/*******************************************************************/

Ping::Ping(const char *hostname_) {
    hostname = hostname_;
}

void Ping::run() {
    struct hostent *host;
    if (nullptr == (host = gethostbyname(hostname))) {
        printf("hostname error.\n");
        usage();
        exit(1);
    }
    hostname = host->h_name;

    // 目标IP
    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.sin_family = PF_INET;
    dst_addr.sin_port = ntohs(0);
    dst_addr.sin_addr = *(struct in_addr *) host->h_addr_list[0];
    // ICMP协议连接
    if ((sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        printf("raw socket created error.\n");
        exit(1);
    }

    pid = getpid();

    // 注册信号处理函数，包括SIGINT结束信号，以及用于持续发ICMP报文的SIGALRM定时信号。
    set_sighandler();

    printf("PING %s (%s): %d bytes data.\n", hostname, inet_ntoa(dst_addr.sin_addr), ICMP_LEN);

    // 1微秒后启动定时器，然后每隔1秒钟发送一个SIGALRM信号，然后重置，继续重复下去
    struct itimerval itv; // 倒计时
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = 1;
    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 20000; // 1秒整会丢包，只能加一点时间...
    if (-1 == (setitimer(ITIMER_REAL, &itv, nullptr))) {
        printf("setitimer fails.\n");
        exit(1);
    }

    // 进入loop，只要接收的报文数量还未到达最大数量，就继续处理响应报文。同时定时器也会继续产生信号，使ICMP请求被发送。
    recv_ping_reply();
}

// 发送ping命令实际执行的ICMP请求包
void Ping::send_icmp_request() {
    char send_buffer[BUFFER_SIZE];
    memset(send_buffer, 0, sizeof(send_buffer));
    struct icmp_header *icmp_hdr = (struct icmp_header *) send_buffer;
    icmp_hdr->icmp_type = ICMP_ECHO_REQUEST; // ICMP请求
    icmp_hdr->icmp_code = 0;
    icmp_hdr->icmp_id = pid;
    icmp_hdr->icmp_seq = ++sent_num; // 按照标准ping程序从1开始显示
    icmp_hdr->icmp_checksum = 0; // 校验和置零
    memset(icmp_hdr->icmp_data, 0, sizeof(icmp_hdr->icmp_data));
    // 发送端发送数据的时间
    gettimeofday((struct timeval *) icmp_hdr->icmp_timestamp, nullptr);

    /**
     * 发送数据时：
     * 1、把校验和字段设置为0；
     * 2、把需要校验的数据看成以16位为单位的数字组成，依次进行二进制求和，然后取反码；
     * 3、把得到的结果存入校验和字段中。
     */
    icmp_hdr->icmp_checksum = compute_checksum((u8 *) icmp_hdr, ICMP_LEN);

    // 原始套接字，用sendto、recvfrom发送和接收
    int res = sendto(sockfd, icmp_hdr, ICMP_LEN, 0,
                     (struct sockaddr *) &dst_addr, sizeof(dst_addr));
    if (-1 == res) {
        printf("sendto fails\n");
    }
}

// 解析发回来的IP响应包
int Ping::parse_ip_reply(char (&recv_buffer)[BUFFER_SIZE]) {
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

    // 发送请求包时的时间
    struct timeval *send_request_time = (struct timeval *) icmp_hdr->icmp_timestamp;
    // 收到响应包时的时间
    struct timeval recv_reply_time;
    gettimeofday(&recv_reply_time, nullptr);

    // RTT(Round-Trip Time): 往返时延，表示从发送端发送数据开始，到发送端收到来自接收端的确认总共经历的时延
    double rtt = ((&recv_reply_time)->tv_sec - send_request_time->tv_sec) * 1000 +
                 ((&recv_reply_time)->tv_usec - send_request_time->tv_usec) / 1000.0;

    printf("%d bytes from %s: icmp_seq=%u ttl=%d rtt=%.1f ms\n",
           icmp_len,
           inet_ntoa(reply_addr.sin_addr),
           icmp_hdr->icmp_seq,
           ip_hdr->ip_ttl,
           rtt);

    return 0;
}

// 接收所有的ping命令响应包
void Ping::recv_ping_reply() {
    /**
     * 由于SIGALRM定时产生且被捕获，因此会一直发送ICMP请求。收到ICMP响应后就会进入解析响应包的函数，直到到达显示的次数。
     * 只要recv_num还没到达最大数量，就继续循环。
     * 因此，大致流程为：
     * SIGALRM信号一直在产生，使得ICMP请求被发送。
     * 而recv_ping_reply函数在main函数中被调用，一被调用就进入while循环，直到处理次数到达最大次数才返回到main中结束进程。
     */
    socklen_t len = sizeof(reply_addr);
    char recv_buffer[BUFFER_SIZE];
    memset(recv_buffer, 0, sizeof(recv_buffer));

    while (recv_num < MAX_ECHO_TIMES) {
        // 有ICMP请求被发送过去过，recv才有数据，否则阻塞，直到响应包到达
        int recv_len = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0,
                                (struct sockaddr *) &reply_addr, &len);

        if (-1 == recv_len) {
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
}

// 计算校验和
u16 Ping::compute_checksum(u8 *buffer, int len) {
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
void Ping::get_statistics() {
    /**
     * const定义的常量在超出其作用域之后其空间会被释放，而static定义的静态常量在函数执行后不会释放其存储空间。
     * 所以，不能在这里把sent_num、recv_num当做实参传入，即使const也不行，否则和static冲突。
     */
    printf("--- %s ping statistics ---\n", hostname);
    printf("%d packets transmitted, %d received, %0.0f%% packet loss\n",
           sent_num, recv_num, 1.0 * (sent_num - recv_num) / recv_num * 100);
}

// 结束
void Ping::end() {
    get_statistics();
    close(sockfd);
}

// 注册信号处理函数
void Ping::set_sighandler() {
    act_alarm.sa_handler = alarm_handler_static; // 静态函数
    if (-1 == sigaction(SIGALRM, &act_alarm, nullptr)) {
        printf("SIGALRM handler setting fails.\n");
        exit(1);
    }

    act_int.sa_handler = int_handler_static; // 静态函数
    if (-1 == sigaction(SIGINT, &act_int, nullptr)) {
        printf("SIGINT handler setting fails.\n");
        exit(1);
    }
}

// SIGINT
void Ping::int_signal_handler(int signo) {
    printf("\n");
    // 信号被捕获时，输出统计信息，然后关闭
    end();
    exit(1);
}

// SIGALRM
void Ping::alarm_signal_handler(int signo) {
    // 定时器设定时间，每隔这个设定的时间就发送一个SIGALRM信号，然后对其捕获，发送ICMP数据包，实现一直打印新的响应报文的效果
    send_icmp_request();
}

/*********************************************************************************************
 * 一个静态方法根本不可能【直接】访问非静态方法，即普通的sortPerThread()，需要用间接的形式
 *
 * e.g. (1)
 * The only way to access non-static elements in a static member function
 * is to tell the function [which object] it has to use for getting access non-static function.
 * class MyClass {
 *     bool eventActive = false;
 *     static bool JoinCommand(MyClass *object) {
 *         // No error, since we are referencing through an object.
 *         if (object->eventActive) {
 *             // do something here...
 *         }
 *     }
 *  };
 *
 *  e.g. (2)
 *  Let the static function give you a [pointer] to the class instance:
 *  class OfApp {
 *  void update(); //non Static Function
 *  static void staticFunction(void * userData) {
 *      auto app = (OfApp*)userData;
 *      app->update();
 *  }
 *
 *  简而言之，就是在静态方法中，用类的【对象指针】去访问非静态成员方法
 *  所以，下面的调用还可以这样写：
 *  Ping *p = nullptr;
 *  m->int_signal_handler();
 *********************************************************************************************
 */
void Ping::int_handler_static(int signo) {
    static_cast<Ping *>(nullptr)->int_signal_handler(signo);
}

void Ping::alarm_handler_static(int signo) {
    static_cast<Ping *>(nullptr)->alarm_signal_handler(signo);
}

Ping::~Ping() {
    end();
}

void usage() {
    printf("[USAGE] sudo ./ping [hostname/IP Address]\n");
}