#include "netif.h"

#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define ROUNDUP2(a, n)       ((a) > 0 ? (1 + (((a) - 1U) | ((n) - 1))) : (n))

#if __APPLE__
    // https://github.com/apple-oss-distributions/network_cmds/blob/8f38231438e6a4d16ef8015e97e12c2c05105644/rtsol.tproj/if.c#L243
    #define ROUNDUP(a)           ROUNDUP2((a), sizeof(uint32_t))
#elif __sun
    // https://github.com/illumos/illumos-gate/blob/95b8c88950fa7b19af46bc63230137cf96b0bff7/usr/src/cmd/cmd-inet/usr.sbin/route.c#L339
    #define ROUNDUP(a)           ROUNDUP2((a), sizeof(long))
#else
    #error unknown platform
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

            #ifndef __sun
            uint32_t salen = sa->sa_len;
            #else
            uint32_t salen;
            // https://github.com/illumos/illumos-gate/blob/95b8c88950fa7b19af46bc63230137cf96b0bff7/usr/src/cmd/cmd-inet/usr.sbin/route.c#L2941
            switch (sa->sa_family) {
            case AF_INET:
                salen = sizeof (struct sockaddr_in);
                break;
            case AF_LINK:
                salen = sizeof (struct sockaddr_dl);
                break;
            case AF_INET6:
                salen = sizeof (struct sockaddr_in6);
                break;
            default:
                salen = sizeof (struct sockaddr);
                break;
            }
            #endif
            sa = (struct sockaddr *)(ROUNDUP(salen) + (char *)sa);
        }
    }
    return NULL;
}

bool ffNetifGetDefaultRouteImplV4(FFNetifDefaultRouteResult* result)
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
            .rtm_addrs = RTA_DST | RTA_IFP | RTA_IFA,
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

    if (send(pfRoute, &rtmsg, rtmsg.hdr.rtm_msglen, 0) != rtmsg.hdr.rtm_msglen)
        return false;

    while (recv(pfRoute, &rtmsg, sizeof(rtmsg), 0) > 0 && !(rtmsg.hdr.rtm_seq == 1 && rtmsg.hdr.rtm_pid == pid))
        ;

    #ifndef __sun // On Solaris, the RTF_GATEWAY flag is not set for default routes for some reason
    if ((rtmsg.hdr.rtm_flags & (RTF_UP | RTF_GATEWAY)) == (RTF_UP | RTF_GATEWAY))
    #endif
    {
        struct sockaddr_dl* sdl = (struct sockaddr_dl *)get_rt_address(&rtmsg.hdr, RTA_IFP);
        if (sdl
            #ifndef __sun
            && sdl->sdl_len
            #endif
        )
        {
            assert(sdl->sdl_nlen <= IF_NAMESIZE);
            memcpy(result->ifName, sdl->sdl_data, sdl->sdl_nlen);
            result->ifName[sdl->sdl_nlen] = '\0';
            result->ifIndex = sdl->sdl_index;

            // Get the preferred source address
            struct sockaddr_in* src = (struct sockaddr_in*)get_rt_address(&rtmsg.hdr, RTA_IFA);
            if (src && src->sin_family == AF_INET)
                result->preferredSourceAddrV4 = src->sin_addr.s_addr;

            return true;
        }
        return false;
    }

    return false;
}

bool ffNetifGetDefaultRouteImplV6(FFNetifDefaultRouteResult* result)
{
    //https://github.com/hashPirate/copenheimer-masscan-fork/blob/36f1ed9f7b751a7dccd5ed27874e2e703db7d481/src/rawsock-getif.c#L104

    FF_AUTO_CLOSE_FD int pfRoute = socket(PF_ROUTE, SOCK_RAW, AF_INET6);
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
        struct sockaddr_in6 dst;
        uint8_t data[512];
    } rtmsg = {
        .hdr = {
            .rtm_type = RTM_GET,
            .rtm_flags = RTF_UP | RTF_GATEWAY,
            .rtm_version = RTM_VERSION,
            .rtm_addrs = RTA_DST | RTA_IFP,
            .rtm_msglen = sizeof(rtmsg.hdr) + sizeof(rtmsg.dst),
            .rtm_pid = pid,
            .rtm_seq = 2,
        },
        .dst = {
            .sin6_family = AF_INET6,
            #ifndef __sun
            .sin6_len = sizeof(rtmsg.dst),
            #endif
        },
    };

    if (send(pfRoute, &rtmsg, rtmsg.hdr.rtm_msglen, 0) != rtmsg.hdr.rtm_msglen)
        return false;

    while (recv(pfRoute, &rtmsg, sizeof(rtmsg), 0) > 0 && !(rtmsg.hdr.rtm_seq == 2 && rtmsg.hdr.rtm_pid == pid))
        ;

    #ifndef __sun // On Solaris, the RTF_GATEWAY flag is not set for default routes for some reason
    if ((rtmsg.hdr.rtm_flags & (RTF_UP | RTF_GATEWAY)) == (RTF_UP | RTF_GATEWAY))
    #endif
    {
        struct sockaddr_dl* sdl = (struct sockaddr_dl *)get_rt_address(&rtmsg.hdr, RTA_IFP);
        if (sdl
            #ifndef __sun
            && sdl->sdl_len
            #endif
        )
        {
            assert(sdl->sdl_nlen <= IF_NAMESIZE);
            memcpy(result->ifName, sdl->sdl_data, sdl->sdl_nlen);
            result->ifName[sdl->sdl_nlen] = '\0';
            result->ifIndex = sdl->sdl_index;

            return true;
        }
        return false;
    }

    return false;
}
