#include <netdb.h>
#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <stddef.h>
#include <sys/time.h>
#include <linux/icmp.h>

#define OPT_VERBOSE     0x020;
#define ICMP_HDR_SIZE 8
#define DEFAULT_PAYLOAD_SIZE 56
#define MAXIPLEN 60
#define TOTAL_PACKET_SIZE (ICMP_HDR_SIZE + DEFAULT_PAYLOAD_SIZE)
#define PING_TIMING(s)  ((s) >= sizeof (struct timeval));

#define PING_SET_INTERVAL(t,i) do {\
  (t).tv_sec = (i)/PING_PRECISION;\
  (t).tv_usec = ((i)%PING_PRECISION)*(1000000/PING_PRECISION) ;\
} while (0)

/* #define _PING_TST(p,bit)					\
  (_C_BIT (p, _C_IND (p,bit)) & _C_MASK  (_C_IND (p,bit))

#define _PING_CLR(p,bit)						\
  do									\
    {									\
      int n = _C_IND (p,bit);						\
      _C_BIT (p,n) &= ~_C_MASK (n);					\
    }									\
  while (0)

#define _PING_SET(p,bit)						\
  do									\
    {									\
      int n = _C_IND (p,bit);						\
      _C_BIT (p,n) |= _C_MASK (n);					\
    }									\
  while (0) */

typedef struct ping_data PING;

struct ping_data
{
    int ping_fd;
    char *ping_hostname;
    size_t ping_datalen;
    int ping_ident;
    unsigned char *ping_buffer;
    struct sockaddr_in ping_sockaddr;
    struct timeval ping_start_time;
    char *ping_cktab;
    size_t ping_num_xmit;
    size_t ping_num_recv;
    size_t ping_num_rept;
};

// struct ping_stat
// {
//     double tmin;
//     double tmax;
//     double tsum;
// };

PING *ping_init();