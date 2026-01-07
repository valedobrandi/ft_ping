#include "../ft_ping.h"

int ping_echo( PING *ping, char *hostname)
{
    struct ping_stat ping_stat;
    int status;

    memset(&ping_stat, 0, sizeof(ping_stat));
    ping_stat.min = 999999999.0;

    if (ping_set_dest(ping, hostname))
    {
        error(EXIT_FAILURE, 0, "unknown host");
    }

    printf("PING %s (%s): %zu data bytes",
        ping->ping_hostname,
        inet_ntoa(ping.ping_sockaddr.sin_addr, DEFAULT_PAYLOAD_SIZE);
    )

    if (options & OPT_VERBOSE)
    {
        printf (", id 0x%04x = %u", ping->ping_ident, ping->ping_ident);
    }

    print("\n");

    status = ping_run(ping);
}