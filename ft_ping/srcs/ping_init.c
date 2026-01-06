#include "../ft_ping.h"
#include <linux/icmp.h>
#include <netinet/ip_icmp.h>

PING *ping_init() {
    int fd;
    struct protoent *proto;
    PING *p;

    proto = getprotobyname("icmp");
    if (!proto)
    {
        fprintf(stderr, "ping:unknow protocol icmp\n");
        return NULL;
    }

    fd = socket(AF_INET, SOCK_RAW, proto->p_proto);
    if (fd < 0)
    {
        return NULL;
    }

    p = malloc(sizeof(*p));
    if (!p)
    {
        pclose(fd);
        return p;
    }

    memset(p, 0, sizeof(*p));
    p->ping_fd = fd;
    //p->ping_datalen = sizeof (icmphdr);
    //p->ping_cktab_size = PING_CKTABSIZE;
    gettimeofday(&p->ping_start_time, NULL);
    return p;
}