#include "ft_ping.h"
#include "parse.h"

void echo(
    struct s_socket_header packet_recv, 
    t_ping *stats, 
    struct sockaddr_in r_addr, 
    ssize_t bytes_received, 
    struct timeval start, 
    t_opts options
)
{
    struct timeval end;
    int type = packet_recv.icmpHeader->type;
    int code = packet_recv.icmpHeader->code;

    if (packet_recv.icmpHeader->type == ICMP_ECHOREPLY)
    {
        unsigned int bytes_expected = bytes_received - (packet_recv.ipHeader->ihl * 4);
        uint16_t recv_seq = ntohs(packet_recv.icmpHeader->un.echo.sequence);

        
        if (ntohs(packet_recv.icmpHeader->un.echo.id) != (getpid() & 0xFFFF))
        {
            return;
        }

        if (recv_seq > stats->transmitted) {
            return; 
        }

        if (verify_checksum(packet_recv.icmpHeader, bytes_expected) == 0)
        {
            fprintf(stderr, "checksum mismatch from %s\n", inet_ntoa(r_addr.sin_addr));
            return;
        }
        stats->received++;
        gettimeofday(&end, NULL);
        double time_elapsed = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    
        printf("%u bytes from %s: icmp_seq=%d ttl=%d time=%.1f ms\n", 
            bytes_expected, 
            inet_ntoa(r_addr.sin_addr), 
            ntohs(packet_recv.icmpHeader->un.echo.sequence), 
            packet_recv.ipHeader->ttl, 
            time_elapsed
        );
    
        if (time_elapsed < stats->rtt_min)
            stats->rtt_min = time_elapsed;
        if (time_elapsed > stats->rtt_max)
            stats->rtt_max = time_elapsed;
        stats->rtt_sum += time_elapsed;
        stats->rtt_sum_squares += time_elapsed * time_elapsed;
    }

    if (options.verbose)
    {
        
        struct iphdr *inner_ip = (struct iphdr *)(packet_recv.payload);
        struct icmphdr *inner_icmp = (struct icmphdr *)((char *)inner_ip + (inner_ip->ihl * 4));
        if (ntohs(inner_icmp->un.echo.id) != (getpid() & 0xFFFF))
        {
            return;
        }
        
        char *err_msg = ft_print_icmp_error(type, code);
        
        printf("From %s icmp_seq=%d %s\n", 
            inet_ntoa(r_addr.sin_addr), 
            ntohs(inner_icmp->un.echo.sequence), 
            err_msg
        );
    }
}