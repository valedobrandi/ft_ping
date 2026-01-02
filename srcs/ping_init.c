#include "ft_ping.h"

ping_init() {
    int fd;
    struct protoent *proto;
    PING *p;

    proto = getprotobyname("icmp")
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
        close(fd);
        return p;
    }

    memset(p, 0, sizeof(*p));
    p->ping_fd = fd;
    p->ping_datalen = sizeof(icmphdr_t);
    p->ping_cktab_size = PING_CKTABSIZE;
    gettimeofday(&p->ping_start_time, NULL);
    return p;
}