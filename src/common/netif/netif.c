#include "netif.h"

#ifndef _WIN32
    #include <net/if.h>
#else
    #define IF_NAMESIZE 16
#endif

bool ffNetifGetDefaultRouteImpl(char iface[IF_NAMESIZE + 1], uint32_t* ifIndex);
enum { IF_INDEX_UNINITIALIZED = (uint32_t) -1, IF_INDEX_INVALID = (uint32_t) -2 };
static uint32_t ifIndex = IF_INDEX_UNINITIALIZED;
static char ifName[IF_NAMESIZE + 1];

static inline void init()
{
    if (ifIndex == (uint32_t) IF_INDEX_UNINITIALIZED && !ffNetifGetDefaultRouteImpl(ifName, &ifIndex))
        ifIndex = (uint32_t) IF_INDEX_INVALID;
}

const char* ffNetifGetDefaultRouteIfName()
{
    init();
    return ifName;
}

uint32_t ffNetifGetDefaultRouteIfIndex()
{
    init();
    return ifIndex;
}
