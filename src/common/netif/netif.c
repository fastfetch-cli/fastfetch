#include "netif.h"

#ifndef _WIN32
    #include <net/if.h>
    #include <netinet/in.h>
#endif

const FFNetifDefaultRouteResult* ffNetifGetDefaultRouteV4(void)
{
    static FFNetifDefaultRouteResult result;
    if (result.status == FF_NETIF_UNINITIALIZED) {
        result.status = ffNetifGetDefaultRouteImplV4(&result) ? FF_NETIF_OK : FF_NETIF_INVALID;
    }
    return &result;
}

const FFNetifDefaultRouteResult* ffNetifGetDefaultRouteV6(void)
{
    static FFNetifDefaultRouteResult result;
    if (result.status == FF_NETIF_UNINITIALIZED) {
        result.status = ffNetifGetDefaultRouteImplV6(&result) ? FF_NETIF_OK : FF_NETIF_INVALID;
    }
    return &result;
}
