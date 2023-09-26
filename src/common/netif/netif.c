#include "netif.h"

#ifndef _WIN32
#include <net/if.h>

bool ffNetifGetDefaultRouteImpl(char iface[IF_NAMESIZE + 1]);

const char* ffNetifGetDefaultRoute()
{
    static char iface[IF_NAMESIZE + 1];

    if (*(uint16_t*) iface == 0)
    {
        if (!ffNetifGetDefaultRouteImpl(iface))
            iface[1] = 1;
    }

    return iface;
}
#else
bool ffNetifGetDefaultRouteImpl(uint32_t* ifIndex);

uint32_t ffNetifGetDefaultRoute()
{
    static uint32_t ifIndex = (uint32_t) -1;

    if (ifIndex == (uint32_t) -1)
    {
        if (!ffNetifGetDefaultRouteImpl(&ifIndex))
            ifIndex = (uint32_t) -2;
    }

    return ifIndex;
}
#endif
