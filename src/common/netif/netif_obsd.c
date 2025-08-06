#include "netif.h"

#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/sysctl.h>

#define ROUNDUP2(a, n)       ((a) > 0 ? (1 + (((a) - 1U) | ((n) - 1))) : (n))

#if defined(__OpenBSD__) || defined(__DragonFly__)
# define ROUNDUP(a)           ROUNDUP2((a), sizeof(int))
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
            sa = (struct sockaddr *)(ROUNDUP(sa->sa_len) + (char *)sa);
        }
    }
    return NULL;
}

bool ffNetifGetDefaultRouteImpl(char iface[IF_NAMESIZE + 1], uint32_t* ifIndex, uint32_t* preferredSourceAddr)
{
    if (preferredSourceAddr)
        *preferredSourceAddr = 0;

    int mib[6] = {CTL_NET, PF_ROUTE, 0, AF_INET, NET_RT_FLAGS, RTF_GATEWAY};
    size_t needed;

    if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0)
        return false;

    FF_AUTO_FREE char* buf = malloc(needed);

    if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0)
        return false;

    char* lim = buf + needed;
    struct rt_msghdr* rtm;
    for (char* next = buf; next < lim; next += rtm->rtm_msglen)
    {
        rtm = (struct rt_msghdr *)next;
        struct sockaddr* sa = (struct sockaddr *)(rtm + 1);

        if ((rtm->rtm_flags & RTF_GATEWAY) && (sa->sa_family == AF_INET))
        {
            struct sockaddr_dl* sdl = (struct sockaddr_dl *)get_rt_address(rtm, RTA_IFP);
            if (sdl->sdl_family == AF_LINK)
            {
                assert(sdl->sdl_nlen <= IF_NAMESIZE);
                memcpy(iface, sdl->sdl_data, sdl->sdl_nlen);
                iface[sdl->sdl_nlen] = '\0';
                *ifIndex = sdl->sdl_index;

                // Get the preferred source address
                struct sockaddr_in* src = (struct sockaddr_in*)get_rt_address(rtm, RTA_IFA);
                if (preferredSourceAddr && src && src->sin_family == AF_INET)
                    *preferredSourceAddr = src->sin_addr.s_addr;

                return true;
            }
        }
    }

    return false;
}
