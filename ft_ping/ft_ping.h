#ifndef FT_PING_H
#define FT_PING_H

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>   
#include <netinet/ip_icmp.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>
#include <math.h>
struct s_socket_header
{
    struct iphdr *ipHeader;
    struct icmphdr *icmpHeader;
    char *payload;
};


typedef struct s_icmp
{
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence;
} t_icmp;

typedef struct PING {
    int transmitted;
    int received;
    double rtt_min;
    double rtt_max;
    double rtt_sum;
    double rtt_sum_squares;
} t_ping;

#endif