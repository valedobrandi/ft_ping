#include <netinet/ip.h>
#include <stddef.h>
#include <netinet/ip_icmp.h>
#include "../ft_ping.h"
#include <linux/icmp.h>
#include <bsd/netinet/ip_icmp.h>

/* icmp_hdr->type

icmp_hdr->code

icmp_hdr->checksum

icmp_hdr->un.echo.id

icmp_hdr->un.echo.sequence */

uint16_t in_cksum(const void *addr, size_t len)
{
    const uint16_t *data = addr;
    uint32_t sum = 0;
    while (len > 1)
    {
        sum += *data++;
        len -= 2;
    }
    
    if (len == 1) {
        uint16_t last = 0;
        *(uint8_t *)&last = *(const uint8_t *)data;
        sum += last;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (u_int16_t)~sum;
}


int icmp_decode(unsigned char *buffer, int n, struct iphdr *ip, struct icmphdr *icmp )
{
    
    ip = (struct iphdr *) buffer;
    icmp = (struct icmphdr *)(buffer + (ip->ihl * 4));
    uint8_t *payload = (uint8_t *)(icmp + 1);
    if (n - (ip->ihl * 4) - sizeof(struct icmphdr) < 8)
        return -1;
    if (n < (ip->ihl * 4))
        return 1;
     return 0;
}

int ping_recv(PING *p)
{
    socklen_t fromlen = sizeof (p->ping_from.ping_sockaddr);
    int n, rc;
    struct icmphdr *icmp;
    struct iphdr *ip;
    int dupflag;

    n = recvfrom(p->ping_fd,
            (char *)p->ping_buffer, 
            (MAXIPLEN + DEFAULT_PAYLOAD_SIZE + ICMP_TSLEN),
            0,
            (struct sockaddr *) &p->ping_from.ping_sockaddr, &fromlen
        );

    if (n < 0) return -1;

    rc = icmp_decode(p->ping_buffer, n, ip, icmp);
    if (rc < 0)
     {
        fprint(stderr, "packet too short (%d bytes) from %s\n",
                n,
                inet_ntoa(p->ping_from.ping_sockaddr.sin_addr));
        return -1;
     }
    switch(icmp->type)
     {
        case ICMP_ECHOREPLY:
        case ICMP_TIMESTAMPREPLY:
        case ICMP_ADDRESSREPLY:

        if ((ntohs(icmp->un.echo.id) != p->ping_ident) && options == OPT_VERBOSE)
         {
             return -1;
         }
        if (rc)
         {
             fprintf(stderr, "checksum mismatch from %s",
                 inet_ntoa(p->ping_from.ping_sockaddr.sin_addr));
                
         }
        p->ping_num_recv++;
        if (_PING_TST(p, ntohs(icmp->un.echo.sequence)))
         {
            p->ping_num_rept++;
            p->ping_num_recv--;
            dupflag = 1;
         }
            else
         {
            _PING_SET(p, ntohs(icmp->un.echo.sequence));
            dupflag = 0;
         }
     }
}