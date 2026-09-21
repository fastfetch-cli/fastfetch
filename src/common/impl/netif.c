#include "common/netif.h"
#include "common/FFcache.h"

#include <string.h>

// netio, localip and dns resolve the default route on every round, and it can change while a
// `--dynamic-interval` run is going on: a laptop switching from Wi-Fi to Ethernet, a VPN coming up.
static FFNetifDefaultRouteResult defaultRouteV4;
static FFNetifDefaultRouteResult defaultRouteV6;

static void resetDefaultRoute(void* storage) {
    // The impls only write the fields they found, so without this a round that finds no default
    // route would keep reporting the interface of the previous one.
    memset(storage, 0, sizeof(FFNetifDefaultRouteResult));
}

static void initDefaultRouteV4(void* storage) {
    FFNetifDefaultRouteResult* result = storage;
    result->status = ffNetifGetDefaultRouteImplV4(result) ? FF_NETIF_OK : FF_NETIF_INVALID;
}

static void initDefaultRouteV6(void* storage) {
    FFNetifDefaultRouteResult* result = storage;
    result->status = ffNetifGetDefaultRouteImplV6(result) ? FF_NETIF_OK : FF_NETIF_INVALID;
}

static FFcacheEntry ffCacheEntryDefaultRouteV4 = {
    .name = "default route V4",
    .storage = &defaultRouteV4,
    .init = initDefaultRouteV4,
    .destroy = resetDefaultRoute,
};

static FFcacheEntry ffCacheEntryDefaultRouteV6 = {
    .name = "default route V6",
    .storage = &defaultRouteV6,
    .init = initDefaultRouteV6,
    .destroy = resetDefaultRoute,
};

const FFNetifDefaultRouteResult* ffNetifGetDefaultRouteV4(void) {
    return ffCacheGet(&ffCacheEntryDefaultRouteV4);
}

const FFNetifDefaultRouteResult* ffNetifGetDefaultRouteV6(void) {
    return ffCacheGet(&ffCacheEntryDefaultRouteV6);
}
