#pragma once

#include "common/option.h"

typedef enum __attribute__((__packed__)) FFLocalIpType
{
    FF_LOCALIP_TYPE_NONE,
    FF_LOCALIP_TYPE_LOOP_BIT        = 1 << 0,
    FF_LOCALIP_TYPE_IPV4_BIT        = 1 << 1,
    FF_LOCALIP_TYPE_IPV6_BIT        = 1 << 2,
    FF_LOCALIP_TYPE_MAC_BIT         = 1 << 3,
    FF_LOCALIP_TYPE_PREFIX_LEN_BIT  = 1 << 4,
    FF_LOCALIP_TYPE_MTU_BIT  = 1 << 5,
    FF_LOCALIP_TYPE_SPEED_BIT  = 1 << 6,
    FF_LOCALIP_TYPE_FLAGS_BIT  = 1 << 7,

    FF_LOCALIP_TYPE_COMPACT_BIT            = 1 << 10,
    FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT = 1 << 11,
    FF_LOCALIP_TYPE_ALL_IPS_BIT            = 1 << 12,
    FF_LOCALIP_TYPE_FORCE_UNSIGNED         = UINT16_MAX,
} FFLocalIpType;
static_assert(sizeof(FFLocalIpType) == sizeof(uint16_t), "");

typedef struct FFLocalIpOptions
{
    FFModuleArgs moduleArgs;

    FFLocalIpType showType;
    FFstrbuf namePrefix;
} FFLocalIpOptions;

static_assert(sizeof(FFLocalIpOptions) <= FF_OPTION_MAX_SIZE, "FFLocalIpOptions size exceeds maximum allowed size");
