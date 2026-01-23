#include "ft_ping.h"
#include "parse.h"

void echo(
    struct s_socket_header packet_recv, 
    t_ping *stats, int seq, 
    struct sockaddr_in r_addr, 
    ssize_t bytes_received, 
    struct timeval start, 
    t_opts options
)
{
    struct timeval end;
    if (packet_recv.icmpHeader->type != ICMP_ECHOREPLY)
    {
        if (options.verbose)
        {
            struct iphdr *inner_ip = (struct iphdr *)(packet_recv.payload);
            struct icmphdr *inner_icmp =
                (struct icmphdr *)((char *)inner_ip + (inner_ip->ihl * 4));
            if (htons(inner_icmp->un.echo.id) != htons(getpid() & 0xFFFF) ||
                inner_icmp->un.echo.sequence != htons(seq))
                return;
            char *err_msg = ft_print_icmp_error(packet_recv.icmpHeader->type, packet_recv.icmpHeader->code);
            printf("From %s icmp_seq=%d %s\n", inet_ntoa(r_addr.sin_addr), seq, err_msg);
        }
        return;
    }
    else if (packet_recv.icmpHeader->type == ICMP_ECHOREPLY &&
            verify_checksum(packet_recv.icmpHeader, bytes_received - (packet_recv.ipHeader->ihl * 4)) == 0)
    {
        fprintf(stderr, "checksum mismatch from %s\n", inet_ntoa(r_addr.sin_addr));
        return;
    }
    stats->received++;

    gettimeofday(&end, NULL);
    double time_elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

    unsigned int bytes_expected = bytes_received - (packet_recv.ipHeader->ihl * 4);
    int ttl = packet_recv.ipHeader->ttl;
    printf("%u bytes from %s: icmp_seq=%d ttl=%d time=%.1f ms\n", bytes_expected, inet_ntoa(r_addr.sin_addr), seq, ttl, time_elapsed);

    if (time_elapsed < stats->rtt_min)
        stats->rtt_min = time_elapsed;
    if (time_elapsed > stats->rtt_max)
        stats->rtt_max = time_elapsed;
    stats->rtt_sum += time_elapsed;
    stats->rtt_sum_squares += time_elapsed * time_elapsed;
}