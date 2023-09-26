#include "netif.h"
#include "common/io/io.h"

#include <net/if.h>
#include <stdio.h>

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

bool ffNetifGetDefaultRouteImpl(char iface[IF_NAMESIZE + 1])
{
    FILE* FF_AUTO_CLOSE_FILE netRoute = fopen("/proc/net/route", "r");
    if (!netRoute) return false;

    // skip first line
    flockfile(netRoute);
    while (getc_unlocked(netRoute) != '\n');
    funlockfile(netRoute);
    unsigned long long destination; //, gateway, flags, refCount, use, metric, mask, mtu,

    while (fscanf(netRoute, "%" FF_STR(IF_NAMESIZE) "s%llx%*[^\n]", iface, &destination) == 2)
    {
        if (destination != 0) continue;
        return true;
    }
    return false;
}
