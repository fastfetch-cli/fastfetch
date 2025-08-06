#include "netif.h"
#include "common/io/io.h"

#include <net/if.h>
#include <stdio.h>

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

static bool getDefaultRouteIPv4(char iface[IF_NAMESIZE + 1], uint32_t* ifIndex, uint32_t* preferredSourceAddr)
{
    if (preferredSourceAddr)
        *preferredSourceAddr = 0;
    // Based on netif_linux.c before 5e770dc8b019702ca458cc0cad46161ebec608a4
    FILE* FF_AUTO_CLOSE_FILE netRoute = fopen("/proc/route", "r");

    if (!netRoute) return false;

    // skip first line
    FF_UNUSED(fscanf(netRoute, "%*[^\n]\n"));

    unsigned long long destination; //, gateway, flags, refCount, use, metric, mask, mtu, ...
    while (fscanf(netRoute, "%" FF_STR(IF_NAMESIZE) "s%llx%*[^\n]", iface, &destination) == 2)
    {
        if (destination != 0) continue;
        *ifIndex = if_nametoindex(iface);
        return true;
    }
    iface[0] = '\0';
    return false;
}

static bool getDefaultRouteIPv6(char iface[IF_NAMESIZE + 1], uint32_t* ifIndex, uint32_t* preferredSourceAddr)
{
    // TODO ipv6
    return false;
}

bool ffNetifGetDefaultRouteImpl(char iface[IF_NAMESIZE + 1], uint32_t* ifIndex, uint32_t* preferredSourceAddr)
{
    if (getDefaultRouteIPv4(iface, ifIndex, preferredSourceAddr))
        return true;

    return getDefaultRouteIPv6(iface, ifIndex, preferredSourceAddr);
}
