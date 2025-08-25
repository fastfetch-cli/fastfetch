#include "netif.h"
#include "common/io/io.h"

#include <net/if.h>
#include <stdio.h>

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

bool ffNetifGetDefaultRouteImplV4(FFNetifDefaultRouteResult* result)
{
    FILE* FF_AUTO_CLOSE_FILE netRoute = fopen("/proc/route", "r");

    if (!netRoute) return false;

    // skip first line
    FF_UNUSED(fscanf(netRoute, "%*[^\n]\n"));
    unsigned long long destination; //, gateway, flags, refCount, use, metric, mask, mtu, ...
    while (fscanf(netRoute, "%" FF_STR(IF_NAMESIZE) "s%llx%*[^\n]", result->ifName, &destination) == 2)
    {
        if (destination != 0) continue;
        result->ifIndex = if_nametoindex(result->ifName);
        // TODO: Get the preferred source address
        return true;
    }
    result->ifName[0] = '0';
    return false;
}

bool ffNetifGetDefaultRouteImplV6(FFNetifDefaultRouteResult* result)
{
    // TODO: AF_INET6
    FF_UNUSED(result);
    return false;
}
