#include "netusage.h"

#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>
#include <sys/sysctl.h>
#include <sys/socket.h>

const char* ffNetUsageGetIoCounters(FFlist* result)
{
    size_t bufSize = 0;
    if (sysctl((int[]) { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST2, 0 }, 6, NULL, &bufSize, 0, 0) < 0)
        return "sysctl({ CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST2, 0 }, 6, NULL, &bufSize, 0, 0) failed";

    FF_AUTO_FREE struct if_msghdr2* buf = (struct if_msghdr2*) malloc(bufSize);
    if (sysctl((int[]) { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST2, 0 }, 6, buf, &bufSize, 0, 0) < 0)
        return "sysctl({ CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST2, 0 }, 6, buf, &bufSize, 0, 0) failed";

    for (struct if_msghdr2* ifm = buf;
        ifm < (struct if_msghdr2*) ((uint8_t*) buf + bufSize);
        ifm = (struct if_msghdr2*) ((uint8_t*) ifm + ifm->ifm_msglen))
    {
        if (ifm->ifm_type != RTM_IFINFO2 || !(ifm->ifm_flags & IFF_RUNNING) || (ifm->ifm_flags & IFF_NOARP)) continue;

        struct sockaddr_dl* sdl = (struct sockaddr_dl*) (ifm + 1);
        assert(sdl->sdl_family == AF_LINK);
        if (sdl->sdl_type != IFT_ETHER && !(ifm->ifm_flags & IFF_LOOPBACK)) continue;
        FFNetUsageIoCounters* counters = (FFNetUsageIoCounters*) ffListAdd(result);

        *counters = (FFNetUsageIoCounters) {
            .txBytes = ifm->ifm_data.ifi_obytes,
            .rxBytes = ifm->ifm_data.ifi_ibytes,
            .txPackets = ifm->ifm_data.ifi_opackets,
            .rxPackets = ifm->ifm_data.ifi_ipackets,
            .txErrors = ifm->ifm_data.ifi_oerrors,
            .rxErrors = ifm->ifm_data.ifi_ierrors,
            .txDrops = 0, // unsupported
            .rxDrops = ifm->ifm_data.ifi_iqdrops,
        };

        ffStrbufInitNS(&counters->name, sdl->sdl_nlen, sdl->sdl_data);
    }

    return NULL;
}
