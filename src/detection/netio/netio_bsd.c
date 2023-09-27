#include "netio.h"

#include "common/io/io.h"
#include "common/netif/netif.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>
#include <sys/sysctl.h>
#include <sys/socket.h>

const char* ffNetIOGetIoCounters(FFlist* result, FFNetIOOptions* options)
{
    uint32_t defaultRouteIfIndex = ffNetifGetDefaultRouteIfIndex();

    size_t bufSize = 0;
    if (sysctl((int[]) { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, (options->defaultRouteOnly ? (int) defaultRouteIfIndex : 0) }, 6, NULL, &bufSize, 0, 0) < 0)
        return "sysctl({ CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, ifIndex }, 6, NULL, &bufSize, 0, 0) failed";

    FF_AUTO_FREE struct if_msghdr* buf = (struct if_msghdr*) malloc(bufSize);
    if (sysctl((int[]) { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, (options->defaultRouteOnly ? (int) defaultRouteIfIndex : 0) }, 6, buf, &bufSize, 0, 0) < 0)
        return "sysctl({ CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, ifIndex }, 6, buf, &bufSize, 0, 0) failed";

    for (struct if_msghdr* ifm = buf;
        ifm < (struct if_msghdr*) ((uint8_t*) buf + bufSize);
        ifm = (struct if_msghdr*) ((uint8_t*) ifm + ifm->ifm_msglen))
    {
        if (ifm->ifm_type != RTM_IFINFO || !(ifm->ifm_flags & IFF_RUNNING) || (ifm->ifm_flags & IFF_NOARP)) continue;

        struct sockaddr_dl* sdl = (struct sockaddr_dl*) (ifm + 1);
        assert(sdl->sdl_family == AF_LINK);
        if (sdl->sdl_type != IFT_ETHER && !(ifm->ifm_flags & IFF_LOOPBACK)) continue;

        sdl->sdl_data[sdl->sdl_nlen] = 0;

        if (options->namePrefix.length && strncmp(sdl->sdl_data, options->namePrefix.chars, options->namePrefix.length) != 0)
            continue;

        FFNetIOResult* counters = (FFNetIOResult*) ffListAdd(result);
        *counters = (FFNetIOResult) {
            .name = ffStrbufCreateNS(sdl->sdl_nlen, sdl->sdl_data),
            .txBytes = ifm->ifm_data.ifi_obytes,
            .rxBytes = ifm->ifm_data.ifi_ibytes,
            .txPackets = ifm->ifm_data.ifi_opackets,
            .rxPackets = ifm->ifm_data.ifi_ipackets,
            .txErrors = ifm->ifm_data.ifi_oerrors,
            .rxErrors = ifm->ifm_data.ifi_ierrors,
            .txDrops = 0, // unsupported
            .rxDrops = ifm->ifm_data.ifi_iqdrops,
            .defaultRoute = sdl->sdl_index == defaultRouteIfIndex,
        };
    }

    return NULL;
}
