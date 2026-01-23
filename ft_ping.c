#include "parse.h"
#include "ft_ping.h"

t_opts options;
int volatile stop = 0;  

void
sig_int (int signal)
{
    (void)signal;
    stop = 1;
}

static t_arg42 opts[] = {
    {NULL,'v', NULL, "Enable verbose output"},
    {NULL, 'c', "[ARG]", "stop after sending NUMBER packets"},
    {NULL, 'i', "[ARG]", "wait NUMBER seconds between sending each ""packet"},
    {"--ttl", 'T', "[ARG]", "specify N as time-to-live"},
    {NULL, 'w', "[ARG]", "stop after N seconds"},
    {NULL, 'W', "[ARG]", "number of seconds to wait for response"},
    {NULL, '?', NULL,"Display help information"},
    {0, 0, 0, 0}
};

static int handle_options(int key, char *arg, void *user)
{
    t_args *args = user;

    switch (key)
    {
    case '?':
        args->help = 1;
        break;
    case 'v':
        options.verbose = 1;
        break;
    case 'c':
        options.count = parse_number(arg, 0, 1);
        break;
    case 'i':
        options.interval = parse_number(arg, 0, 1);
        options.has_interval = 1;
        break;
    case 'T':
        options.ttl = parse_number(arg, 255, 1);
        break;
    case 'w':
        options.timeout = parse_number(arg, MAX_INTERVAL, 0);
        break;
    case 'W':
        options.linger = parse_number(arg, MAX_INTERVAL, 0);
        break;
    case 0:
        if (args->host)
            break;
        args->host = arg;
        break;
    default:
        return 1;
    }
    return 0;
}


// Simple checksum function
unsigned short checksum(void *b, int len) {
    uint16_t *buf = b;
    uint32_t sum = 0;
    for (sum = 0; len > 1; len -= 2) 
        sum += *buf++;
    if (len == 1) 
        sum += *(uint8_t *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (uint16_t)(~sum);
}

struct s_socket_header parse_header(char *buffer)
{
    struct iphdr *ipHeader = (struct iphdr *)buffer;
    struct icmphdr *icmp = (struct icmphdr *)(buffer + (ipHeader->ihl * 4));
    char *payload = buffer + (ipHeader->ihl * 4) + sizeof(struct icmphdr);

    struct s_socket_header header = {ipHeader, icmp, payload};
    return header;
}

int build_packet(char *buffer, int seq) {
    // Build ICMP Header
    struct s_icmp *icmp = (struct s_icmp *)buffer;
    memset(icmp, 0, sizeof(*icmp));
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->identifier= htons(getpid() & 0xFFFF);
    icmp->sequence = htons(seq);
    
    memset(buffer + sizeof(*icmp), 0, 56);

    size_t packet = sizeof(*icmp) + 56;
    icmp->checksum = 0;
    icmp->checksum = checksum(buffer, packet);
    return packet;
}

int verify_checksum(struct icmphdr *icmp, int len) {
    uint16_t received_checksum = icmp->checksum;
    icmp->checksum = 0;
    uint16_t calculated_checksum = checksum(icmp, len);
    icmp->checksum = received_checksum;
    return received_checksum == calculated_checksum;
}

void
finish(t_ping *stats, char *host)
{
    printf("--- %s ping statistics ---\n", host);
    printf("%d packets transmitted, %d received, %.0f%% packet loss\n",
           stats->transmitted,
           stats->received,
           ((stats->transmitted - stats->received) / (double)stats->transmitted) * 100.0);
    if (stats->received > 0) {
        double rtt_avg = stats->rtt_sum / stats->received;
        double rtt_mdev = sqrt((stats->rtt_sum_squares / stats->received) - (rtt_avg * rtt_avg));
        printf("round-trip min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
               stats->rtt_min, rtt_avg, stats->rtt_max,rtt_mdev
            );
    }
}

int main(int argc, char **argv) {
    int sockfd;
    t_ping stats = {0, 0, 1e9, 0, 0, 0};
    char buffer[1024];

    t_args args = {0};
    t_arg42_args main_args = {argc, argv};
    memset(&options, 0, sizeof(options));

    signal (SIGINT, sig_int);

    if (ft_argp(&main_args, opts, handle_options, &args) != 0 || args.help) {
            print_help(opts); return 0;
    }

    if (!args.host) {
        fprintf(stderr, "Host not specified.\n");
        return 1;
    }

    // Prepare Destination 
    struct sockaddr_in dest;
    struct addrinfo addressInfoHints, *addrInfoPointer;
    memset(&addressInfoHints, 0, sizeof(addressInfoHints));
    addressInfoHints.ai_family = AF_INET;
    addressInfoHints.ai_socktype = SOCK_RAW;

    int getaddrinfo_result = getaddrinfo(args.host, NULL, &addressInfoHints, &addrInfoPointer);
    if (getaddrinfo_result != 0)
    {
        fprintf(stderr, "ft_ping: unknown host\n");
        exit(1);
    }
    
    dest = *(struct sockaddr_in *)addrInfoPointer->ai_addr;
    freeaddrinfo(addrInfoPointer);
    
    // 1. Create Raw Socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Socket error");
        return 1;
    }

    if (options.ttl > 0) {
        if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &options.ttl, sizeof(options.ttl)) < 0)
        {
            perror("setsockopt IP_TTL"); exit(1);
        }
    }

    int packet_size = build_packet(buffer, 1);

    // 4. Send ICMP Echo Request
    uint16_t seq = 1;
    
    printf("PING %s (%s): %d data bytes", args.host, inet_ntoa(dest.sin_addr), 56);
    if (options.verbose)
    {
        uint16_t id = getpid() & 0xFFFF;
        printf(", id 0x%04x = %u\n", id, id);
    }
    else
    {
        printf("\n");
    }
   
    // Start timing
    struct timeval timeout;
    gettimeofday(&timeout, NULL);

    while (stop == 0)
    {
        struct timeval start, now;
        gettimeofday(&start, NULL);
        gettimeofday(&now, NULL);

        double current_time = (now.tv_sec - timeout.tv_sec) + (now.tv_usec - timeout.tv_usec) / 1000000.0;
        if (options.timeout > 0 && current_time >= options.timeout) {
            break;
        }
        
        struct timeval packet_timeout;
        packet_timeout.tv_sec = options.linger > 0 ? options.linger : 3;
        packet_timeout.tv_usec = 0;
        // Send ICMP Echo Request
        if (sendto(sockfd, buffer, packet_size, 0, (struct sockaddr*)&dest, sizeof(dest)) <= 0) {
            perror("Send error");
        } else {
            stats.transmitted++;

            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(sockfd, &rfds);
            

            int retval = select(sockfd + 1, &rfds, NULL, NULL, &packet_timeout);
            if (retval < 0) {
                continue;
            } else if (retval == 0) {
                if (options.verbose)
                {
                    fprintf(stderr, "Request timeout for icmp_seq %d\n", seq);
                }
            }
            seq++;

            if (retval > 0)
            {
                struct sockaddr_in r_addr;
                socklen_t len = sizeof(r_addr);
                ssize_t bytes_received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&r_addr, &len);
     
                if (bytes_received < 0)
                {
                    perror("Receive error");
                    continue;
                }
    
                struct s_socket_header packet_recv = parse_header(buffer);
                echo(packet_recv, 
                    &stats, seq - 1, 
                    r_addr, 
                    bytes_received, 
                    start, 
                    options
                );
                
            }
        }
        packet_size = build_packet(buffer, seq);

        sleep(options.has_interval ? options.interval : 1);
   
        if (options.count > 0 && options.count == (unsigned long)stats.transmitted) 
            break;
    }

    close(sockfd);
    finish(&stats, args.host);
    return 0;
}