#include "netio.h"

#include "common/netif/netif.h"
#include "util/mallocHelper.h"

#include <net/if.h>
#include <net/if_mib.h>
#include <sys/sysctl.h>

const char* ffNetIOGetIoCounters(FFlist* result, FFNetIOOptions* options)
{
    int mib[] = {CTL_NET, PF_LINK, NETLINK_GENERIC,
        options->defaultRouteOnly ? IFMIB_IFDATA : IFMIB_IFALLDATA,
        options->defaultRouteOnly ? (int) ffNetifGetDefaultRouteV4()->ifIndex : 0,
        IFDATA_GENERAL};

    size_t bufSize = 0;
    if (sysctl(mib, ARRAY_SIZE(mib), NULL, &bufSize, 0, 0) < 0)
        return "sysctl(mib, ARRAY_SIZE(mib), NULL, &bufSize, 0, 0) failed";

    assert(bufSize % sizeof(struct ifmibdata) == 0);

    FF_AUTO_FREE struct ifmibdata* buf = (struct ifmibdata*) malloc(bufSize);
    if (sysctl(mib, ARRAY_SIZE(mib), buf, &bufSize, 0, 0) < 0)
        return "sysctl(mib, ARRAY_SIZE(mib), buf, &bufSize, 0, 0) failed";

    size_t ifCount = bufSize / sizeof(struct ifmibdata);

    const char* defaultRouteIfName = ffNetifGetDefaultRouteV4()->ifName;

    for (size_t i = 0; i < ifCount; i++)
    {
        struct ifmibdata* mibdata = &buf[i];
        if (!(mibdata->ifmd_flags & IFF_RUNNING) || (mibdata->ifmd_flags & IFF_NOARP))
            continue;

        if (options->namePrefix.length && strncmp(mibdata->ifmd_name, options->namePrefix.chars, options->namePrefix.length) != 0)
            continue;

        FFNetIOResult* counters = (FFNetIOResult*) ffListAdd(result);
        *counters = (FFNetIOResult) {
            .name = ffStrbufCreateS(mibdata->ifmd_name),
            .txBytes = mibdata->ifmd_data.ifi_obytes,
            .rxBytes = mibdata->ifmd_data.ifi_ibytes,
            .txPackets = mibdata->ifmd_data.ifi_opackets,
            .rxPackets = mibdata->ifmd_data.ifi_ipackets,
            .txErrors = mibdata->ifmd_data.ifi_oerrors,
            .rxErrors = mibdata->ifmd_data.ifi_ierrors,
            .txDrops = mibdata->ifmd_snd_drops,
            .rxDrops = mibdata->ifmd_data.ifi_iqdrops,
            .defaultRoute = strncmp(mibdata->ifmd_name, defaultRouteIfName, IFNAMSIZ) == 0,
        };
    }

    return NULL;
}
