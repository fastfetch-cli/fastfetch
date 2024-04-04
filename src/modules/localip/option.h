#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef enum FFLocalIpType
{
    FF_LOCALIP_TYPE_NONE,
    FF_LOCALIP_TYPE_LOOP_BIT        = 1 << 0,
    FF_LOCALIP_TYPE_IPV4_BIT        = 1 << 1,
    FF_LOCALIP_TYPE_IPV6_BIT        = 1 << 2,
    FF_LOCALIP_TYPE_MAC_BIT         = 1 << 3,
    FF_LOCALIP_TYPE_PREFIX_LEN_BIT  = 1 << 4,

    FF_LOCALIP_TYPE_COMPACT_BIT     = 1 << 10,
} FFLocalIpType;

typedef struct FFLocalIpOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

    FFLocalIpType showType;
    FFstrbuf namePrefix;
    bool defaultRouteOnly;
} FFLocalIpOptions;
