#include "ping.h"

void usage();

int main(int argc, char **argv) {
    if (argc != 2) {
        usage();
        exit(1);
    }

    struct hostent *host;
    if (nullptr == (host = gethostbyname(argv[1]))) {
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
    struct itimerval itv;
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = 1;
    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;
    if (-1 == (setitimer(ITIMER_REAL, &itv, nullptr))) {
        printf("setitimer fails.\n");
        exit(1);
    }

    // 进入loop，只要接收的报文数量还未到达最大数量，就继续处理响应报文。同时定时器也会继续产生信号，使ICMP请求被发送。
    recv_ping_reply();

    exit(0);
}

void usage() {
    printf("[USAGE] sudo ./ping [hostname/IP Address]\n");
}