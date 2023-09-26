#include "netif.h"
#include "common/io/io.h"

#include <stdio.h>

bool ffNetifGetDefaultRoute(FFstrbuf* iface)
{
    FILE* FF_AUTO_CLOSE_FILE netRoute = fopen("/proc/net/route", "r");
    if (!netRoute) return false;

    // skip first line
    flockfile(netRoute);
    while (getc_unlocked(netRoute) != '\n');
    funlockfile(netRoute);
    unsigned long long destination; //, gateway, flags, refCount, use, metric, mask, mtu,

    char buf[16 /*IF_NAMESIZE*/ + 1];
    while (fscanf(netRoute, "%16s%llx%*[^\n]", buf, &destination) == 2)
    {
        if (destination != 0) continue;
        ffStrbufSetS(iface, buf);
        return true;
    }
    return false;
}
