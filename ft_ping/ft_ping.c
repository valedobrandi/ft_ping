#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <unistd.h>

// Simple checksum function
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    for (sum = 0; len > 1; len -= 2) sum += *buf++;
    if (len == 1) sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main() {
    int sockfd;
    struct icmphdr header;
    struct sockaddr_in addr;
    char buffer[1024];

    // 1. Create Raw Socket (Requires sudo)
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Socket error");
        return 1;
    }

    // 2. Prepare Destination (e.g., Google DNS 8.8.8.8)
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "8.8.8.8", &addr.sin_addr);

    // 3. Build ICMP Header
    header.type = ICMP_ECHO;
    header.code = 0;
    header.un.echo.id = getpid();
    header.un.echo.sequence = 1;
    header.checksum = 0;
    header.checksum = checksum(&header, sizeof(header));

    // 4. Send and Receive
    if (sendto(sockfd, &header, sizeof(header), 0, (struct sockaddr*)&addr, sizeof(addr)) <= 0) {
        perror("Send error");
    } else {
        printf("Sent ICMP Echo Request to 8.8.8.8...\n");
        
        struct sockaddr_in r_addr;
        socklen_t len = sizeof(r_addr);
        if (recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&r_addr, &len) > 0) {
            printf("Received Echo Reply from %s!\n", inet_ntoa(r_addr.sin_addr));
        }
    }

    close(sockfd);
    return 0;
}