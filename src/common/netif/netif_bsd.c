#include "netif.h"

#include "common/io/io.h"

#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define ROUNDUP2(a, n)       ((a) > 0 ? (1 + (((a) - 1U) | ((n) - 1))) : (n))

#if defined(__APPLE__)
# define ROUNDUP(a)           ROUNDUP2((a), sizeof(int))
#elif defined(__NetBSD__)
# define ROUNDUP(a)           ROUNDUP2((a), sizeof(uint64_t))
#elif defined(__FreeBSD__)
# define ROUNDUP(a)           ROUNDUP2((a), sizeof(int))
#elif defined(__OpenBSD__)
# define ROUNDUP(a)           ROUNDUP2((a), sizeof(int))
#elif defined(__sun)
# define ROUNDUP(a)           ROUNDUP2((a), _SS_ALIGNSIZE)
#else
# error unknown platform
#endif

static struct sockaddr *
get_rt_address(struct rt_msghdr *rtm, int desired)
{
    struct sockaddr *sa = (struct sockaddr *)(rtm + 1);

    for (int i = 0; i < RTAX_MAX; i++)
    {
        if (rtm->rtm_addrs & (1 << i))
        {
            if ((1 << i) == desired)
                return sa;
#ifdef __sun
            sa = (struct sockaddr *)(ROUNDUP(sizeof(struct sockaddr)) + (char *)sa);
#else
            sa = (struct sockaddr *)(ROUNDUP(sa->sa_len) + (char *)sa);
#endif
        }
    }
    return NULL;
}

bool ffNetifGetDefaultRouteImpl(char iface[IF_NAMESIZE + 1], uint32_t* ifIndex)
{
    //https://github.com/hashPirate/copenheimer-masscan-fork/blob/36f1ed9f7b751a7dccd5ed27874e2e703db7d481/src/rawsock-getif.c#L104

    FF_AUTO_CLOSE_FD int pfRoute = socket(PF_ROUTE, SOCK_RAW, AF_INET);
    if (pfRoute < 0)
        return false;

    {
        struct timeval timeout = {1, 0};
        setsockopt(pfRoute, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
        setsockopt(pfRoute, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
    }

    int pid = getpid();

    struct {
        struct rt_msghdr hdr;
        struct sockaddr_in dst;
        uint8_t data[512];
    } rtmsg = {
        .hdr = {
            .rtm_type = RTM_GET,
            .rtm_flags = RTF_UP | RTF_GATEWAY,
            .rtm_version = RTM_VERSION,
            .rtm_addrs = RTA_DST | RTA_IFP,
            .rtm_msglen = sizeof(rtmsg.hdr) + sizeof(rtmsg.dst),
            .rtm_pid = pid,
            .rtm_seq = 1,
        },
        .dst = {
            .sin_family = AF_INET,
            #ifndef __sun
            .sin_len = sizeof(rtmsg.dst),
            #endif
        },
    };

    if (write(pfRoute, &rtmsg, rtmsg.hdr.rtm_msglen) != rtmsg.hdr.rtm_msglen)
        return false;

    while (read(pfRoute, &rtmsg, sizeof(rtmsg)) > 0)
    {
        if (rtmsg.hdr.rtm_seq == 1 && rtmsg.hdr.rtm_pid == pid)
        {
            struct sockaddr_dl* sdl = (struct sockaddr_dl *)get_rt_address(&rtmsg.hdr, RTA_IFP);
            if (sdl)
            {
                assert(sdl->sdl_nlen <= IF_NAMESIZE);
                memcpy(iface, sdl->sdl_data, sdl->sdl_nlen);
                iface[sdl->sdl_nlen] = '\0';
                *ifIndex = sdl->sdl_index;
                return true;
            }
            return false;
        }
    }
    return false;
}
