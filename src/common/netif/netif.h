#pragma once

#include "fastfetch.h"

#ifndef _WIN32
    #include <net/if.h>
    #include <netinet/in.h>
#endif

typedef enum __attribute__((__packed__)) FFNetifDefaultRouteResultStatus {
    FF_NETIF_UNINITIALIZED,
    FF_NETIF_INVALID,
    FF_NETIF_OK
} FFNetifDefaultRouteResultStatus;

typedef struct FFNetifDefaultRouteResult {
    uint32_t ifIndex;

    #ifndef _WIN32
    char ifName[IF_NAMESIZE + 1];
    uint32_t preferredSourceAddrV4;
    #endif
    enum FFNetifDefaultRouteResultStatus status;
} FFNetifDefaultRouteResult;

bool ffNetifGetDefaultRouteImplV4(FFNetifDefaultRouteResult* result);
bool ffNetifGetDefaultRouteImplV6(FFNetifDefaultRouteResult* result);

const FFNetifDefaultRouteResult* ffNetifGetDefaultRouteV4(void);
const FFNetifDefaultRouteResult* ffNetifGetDefaultRouteV6(void);
