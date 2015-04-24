#ifndef _COMMON_SOCK_UTLIS_H
#define _COMMON_SOCK_UTLIS_H
/*
*
* ip address for ipv6 and ipv4.
*
*/
struct ip_addr{
    unsigned int af;     /* address family: AF_INET6 or AF_INET */
    unsigned int len;    /* address len, 16 or 4 */

    /* 64 bits aligned address */
    union {
        unsigned long  addrl[16/sizeof(long)]; /* long format*/
        unsigned int   addr32[4];
        unsigned short addr16[8];
        unsigned char  addr[16];
    }u;
};

/*
* socket address union for ipv6 and ipv4.
*
*/
union sockaddr_union{
    struct sockaddr     s;
    struct sockaddr_in  sin;
    struct sockaddr_in6 sin6;
    struct sockaddr_un  sun;
};

#define TCP 6
#define UDP 17
#define PROTOCOL(p) (((p)==TCP)? SOCK_STREAM : SOCK_DGRAM)
#define FD_NONE     0
#define FD_READABLE 1
#define FD_WRITABLE 2
#define FD_RWERROR  4
#define FD_WAIT_MS 500
#define FD_SET_SO_RCVTIMEO 1
#define FD_SET_SO_SNDTIMEO 2
#define TIMER_TICKS_HZ    16U
#define TIMER_S_TO_MS 1000
#define MS_TO_TICKS(m)  (((m)*TIMER_TICKS_HZ+999U)/1000U)
#define TICKS_TO_MS(t) (((t)*1000U)/TIMER_TICKS_HZ)

// iovec init
#define INIT_IOV(i)        \
struct iovec iovs[i];      \
    int    iovs_count = 0;

#define SET_IOV(y)                                            \
{                                                             \
    iovs[iovs_count].iov_base = (y == NULL) ? "" : (void*)y;  \
    iovs[iovs_count].iov_len  = strlen((y == NULL) ? "" : y); \
    iovs_count++;                                             \
}

#define SET_IOV_LEN(s, l)          \
{                                  \
    iovs[iovs_count].iov_base = const_cast<char *>(s); \
    iovs[iovs_count].iov_len  = l; \
    iovs_count++;                  \
}

#endif  // _COMMON_SOCK_UTLIS_H
