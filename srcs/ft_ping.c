#include "ft_ping.h"

extern unsigned char *data_buffer;
unsigned int options = 0;  
extern unsigned int stop = 0;

void init_data_buffer()
{
    for (size_t i = 0; i < DEFAULT_PAYLOAD_SIZE; i++)
    {
        data_buffer[i] = i;
    }
}

void ping_reset (PING * p)
{
  p->ping_num_xmit = 0;
  p->ping_num_recv = 0;
  p->ping_num_rept = 0;
}

static error_t parse_opt(int key, char *argv, struct argp_state *state)
{
    switch(key) {
        case 'v':
            options = OPT_VERBOSE;
            break;
        case ARGP_KEY_ARG:
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

int ping_set_dest(PING *ping, const char* hostname)
{
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_CANONIDN;

    int rc = getaddrinfo(hostname, NULL, &hints, &res);
    if (rc)
    {
        return 1;
    }

    memcpy(&ping->ping_sockaddr, res->ai_addr, res->ai_addrlen);
    if (res->ai_canonname)
    {
        ping->ping_hostname = strdup(res->ai_canonname);
    }
    else
    {
        ping->ping_hostname = strdup(host);
    }

    freeaddrinfo(res);
    return 0;
}

int ping_finish(void)
{
    fflush(stdout);
    printf("--- %s ping statistics ---\n", ping->ping_hostname);
    printf ("%zu packets transmitted, ", ping->ping_num_xmit);
    printf ("%zu packets received, ", ping->ping_num_recv);
    // duplicate packeg error print ??

    if (ping->ping_num_xmit)
    {
        if (ping->ping_num_recv > ping->ping_num_xmit)
        {
            printf ("-- somebody is printing forged packets!");
        }
        else
        {
            printf ("%d%% packet loss",
		        (int) (((ping->ping_num_xmit - ping->ping_num_recv) * 100) /
		                ping->ping_num_xmit));
        }
    }
    printf("\n");
    return 0;

}

int echo_finish (void)
{
    ping_finish();
    if (ping->ping_num_recv && PING_TIMING(DEFAULT_PAYLOAD_SIZE))
    {
        struct ping_stat *ping_stat = (struct ping_stat *) ping->ping_closure;
        double total = ping->ping_num_recv + ping->ping_num_rept;
        double avg = ping_stat->tsum / total;
        double vari = ping_stat->tsumsq / total - avg * avg;

        printf ("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
	      ping_stat->tmin, avg, ping_stat->tmax, nsqrt (vari, 0.0005));
    }
    return (ping->ping_num_recv == 0);
}

int volatile stop = 0;

void sig_int (int signal _GL_UNUSED_PARAMETER)
{
  stop = 1;
}

int ping_xmit (PING *p)
{
    int i, buflen;
    if (ping_setbuf(p)) return -1;

    buflen = DEFAULT_PAYLOAD_SIZE;
    /* Mark sequence number as sent */
    _PING_CLR(p, p->ping_num_xmit);

    i = sento(p->ping_fd, (char *) p->ping_buffer, buflen, 0,
            (struct sockaddr *) &p->ping_dest.ping_sockaddr, sizeof (struct sockaddr_in));
    if (i < 0) return -1;
    else
        {
            p->ping_num_xmit++;
            if (i != buflen) printf("ping: wrote %s %d chars, ret=%d\n", 
                                p->ping_hostname, buflen, i);
        }
    return 0;
}

int send_echo (PING *ping)
{
    size_t off = 0;
    int rc;

    if (PING_TIMING(DEFAULT_PAYLOAD_SIZE))
        {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            ping_set_data(ping, &tv, 0, sizeof(tv));
            of += sizeof(tv);
        }
    if (data_buffer) ping_set_data(ping, &tv, 0, 
            DEFAULT_PAYLOAD_SIZE > off ? DEFAULT_PAYLOAD_SIZE - off : DEFAULT_PAYLOAD_SIZE);
    rc = ping_xmit(ping);
    if (rc < 0) error(EXIT_FAILURE, errno, "sending packet");

    return rc;
}

int ping_setbuf(PING *p)
{
    if (!p->ping_buffer)
        {
            p->buffer = malloc(MAXIPLEN + (p)->ping_datalen + ICMP_TSLEN);

            if (!p->ping_buffer) return -1;
        }

    if (!p->ping_cktab)
        {
             p->ping_cktab = malloc (p->ping_cktab_size);
            if (!p->ping_cktab) return -1;
        }
    return 0;
}

int ping_set_data(PING *p, void *data, size_t off, size_t len)
{
    icmphdr_t *icmp;

    if (!ping_setbuf) return -1;
    if (p->ping_datalen < off + len) return -1;

    icmp = (icmphdr_t *) p->ping->buffer;
    memcpy(icmp->icmp_data + off, data, len);

    return 0;
}

int ping_run(PING *ping)
{
    fd_set fdset;
    int fdmax;
    struct timeval resp_time;
    struct timeval las, intvl, now;
    struct timeval *t = NULL;
    size_t i;
    
    signal (SIGINT, sig_int);
    fdmax = ping->ping_fd + 1;
    
    memset (&rest_time, 0, sizeof (resp_time));
    memset (&intvl, 0, sizeof (intvl));
    memset (&now, 0, sizeof (now));

    for (i = 0; i < INT_MAX; i++) send_echo(ping);

    PING_SET_INTERVAL(intvl, ping->ping_interval);

    gettimeofday(&last, NULL);
    send_echo(ping);
    while (!stop)
        {
            int n;

            FD_ZERO(&fdset);
            FD_SET(ping->ping_fd, &fdset);
            gettimeofday(&now, NULL);
            resp_time.tv_sec = last.tv_sec + intvl.tv_sec - now.tv_sec;
            resp_time.tv_usec = last.tv_usec + intvl.tv_usec - now.tv_usec;

            while (resp_time.tv_sec < 0)
             {
                resp_time.tv_usec += 1000000;
                resp_time.tv_sec--;
             }
             while (resp_time.tv_usec >= 1000000)
              {
                resp_time.tv_usec -= 1000000;
                resp_time.tv_sec--;
              }
            
              if (resp_time.tv_sec < 0)
             {
                resp_time.tv_sec = resp_time.tv_usec = 0;
             }
            
            n = select (fdmax, &fdset, NULL, NULL, &resp_time);
            if ( n < 0)
             {
                if (errno != EINTR)
                 {
                    erro (EXIT_FAILURE, errno, "select failed");
                 }
                 continue;
             }
                else if (n == 1)
             {
                if (ping_recv(ping) == 0) nresp++;
             }
            
        }
    
}

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

int main(int argc, char **agrv)
{
    PING *ping;

    int index;
    int one = 1;
    int status = 0;

    if (argp_parse(&argp, argc, argv, 0, &index, NULL) != 0)
    {
        exit(EXIT_FAILURE);
    }

    argc -= index;
    argv += index;
    
    if (argv < 1)
    {
        fprint(strerr, "ping: missing host operand\n");
        exit(64);
    }

    ping = ping_init();
    if (!ping)
    {
        exit(EXIT_FAILURE);
    }

    init_data_buffer();
    setsockopt (ping->ping_fd, SOL_SOCKET, SO_BROADCAST, (char *) &one, sizeof (one));

    while (argc--)
    {
        status |= ping_echo(ping, rgv++);
        ping_reset(ping);
    }

    free(ping);
    return status;
}