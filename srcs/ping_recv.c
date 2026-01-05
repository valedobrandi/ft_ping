#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

int icmp_generic_decod(unsigned char *buffer, int n, struct *ip, icmphdr *icmp)
{
    ip = (struct *ip) buffer;
    icmp = (struct *icmp)(buffer + (ip->ihl * 4));
}

int ping_recv(PING *p)
{
    socklen_t fromlen = sizeof (p->ping_from.ping_sockaddr);
    int n, rc;
    icmphdr *icmp;
    struc ip *ip;
    int dupflag;

    n = recvfrom(p->ping_fd,
            (char *)p->ping_buffer, 
            (MAXIPLEN + (p)->ping_datalen + ICMP_TSLEN),
            0,
            (struct sockaddr *) &p->ping_from.ping_sockaddr, &fromlen
        );

    if (n < 0) return -1;
    
    rc = icmp_generic_decode(p->ping_buffer, &ip, &icmp);
    if (rc < 0)
     {
        fprint(stderr, "packet too short (%d bytes) from %s\n",
                n,
                inet_ntoa(p->ping_from.ping_sockaddr.sin_addr));
        return -1;
     }
    switch(icmp->icmp_type)
     {
        case ICMP_ECHOREPLY;
        case ICMP_TIMESTAMPREPLY;
        case ICMP_ADDRESSREPLY;

        if (ntohs(icmp->icmp_id != ping->ping_ident && useless_ident == 0))
         {
             return -1;
         }
        if (rc)
         {
             printf(stderr, "checksum mesmatch from %s",
                 inet_ntoa(p->ping_from.ping_sockaddr.sin_addr));
                
         }
        p->ping_num_recv++;
        if (_PING_TST(p, ntohs(icmp->icmp_seq)))
         {
            p->ping_num_rept++;
            p->ping_num_recv--;
            dupflag = 1;
         }
            else
         {

         }
     }
}