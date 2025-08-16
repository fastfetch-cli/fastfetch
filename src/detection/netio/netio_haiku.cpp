extern "C" {
#include "netio.h"
#include "common/netif/netif.h"
}

#include <NetworkInterface.h>
#include <NetworkRoster.h>

const char* ffNetIOGetIoCounters(FFlist* result, FFNetIOOptions* options)
{
    BNetworkRoster& roster = BNetworkRoster::Default();

    BNetworkInterface interface;
    uint32 cookie = 0;

    uint32_t defaultRouteIfIndex = ffNetifGetDefaultRouteV4()->ifIndex;

    while (roster.GetNextInterface(&cookie, interface) == B_OK)
    {
        if (!interface.Exists())
            continue;

        bool defaultRoute = interface.Index() == defaultRouteIfIndex;
        if (options->defaultRouteOnly && !defaultRoute)
            continue;

        if (options->namePrefix.length && strncmp(interface.Name(), options->namePrefix.chars, options->namePrefix.length) != 0)
            continue;

        ifreq_stats stats = {};
        if (interface.GetStats(stats) != B_OK) continue;

        FFNetIOResult* counters = (FFNetIOResult*) ffListAdd(result);
        *counters = (FFNetIOResult) {
            .name = ffStrbufCreateS(interface.Name()),
            .defaultRoute = defaultRoute,
            .txBytes = stats.send.bytes,
            .rxBytes = stats.receive.bytes,
            .txPackets = stats.send.packets,
            .rxPackets = stats.receive.packets,
            .rxErrors = stats.receive.errors,
            .txErrors = stats.send.errors,
            .rxDrops = stats.receive.dropped,
            .txDrops = stats.send.dropped
        };
    }

    return NULL;
}
