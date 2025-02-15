extern "C" {
#include "netio.h"

#include "common/netif/netif.h"
#include "util/mallocHelper.h"
}

#include <NetworkInterface.h>
#include <NetworkRoster.h>

const char* ffNetIOGetIoCounters(FFlist* result, FFNetIOOptions* options)
{
    BNetworkRoster& roster = BNetworkRoster::Default();

    BNetworkInterface interface;
    uint32 cookie = 0;

    const char* defaultRouteIfName = ffNetifGetDefaultRouteIfName();

    while (roster.GetNextInterface(&cookie, interface) == B_OK)
    {
        if (!interface.Exists())
            continue;

        if (options->defaultRouteOnly && strcmp(interface.Name(), defaultRouteIfName) != 0)
            continue;

        if (options->namePrefix.length && strncmp(interface.Name(), options->namePrefix.chars, options->namePrefix.length) != 0)
            continue;

        ifreq_stats stats;
        memset(&stats, 0, sizeof(stats));
        interface.GetStats(stats);

        FFNetIOResult* counters = (FFNetIOResult*) ffListAdd(result);
        *counters = (FFNetIOResult) {
            .name = ffStrbufCreateS(interface.Name()),
            .defaultRoute = strcmp(interface.Name(), defaultRouteIfName) == 0,
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
