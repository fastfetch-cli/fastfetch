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

typedef enum __attribute__((__packed__)) FFLocalIpIpv6Type
{
    FF_LOCALIP_IPV6_TYPE_NONE          = 0b00000000,
    FF_LOCALIP_IPV6_TYPE_GUA_BIT       = 0b00000001,
    FF_LOCALIP_IPV6_TYPE_ULA_BIT       = 0b00000010,
    FF_LOCALIP_IPV6_TYPE_LLA_BIT       = 0b00000100,
    FF_LOCALIP_IPV6_TYPE_UNKNOWN_BIT   = 0b00001000, // IPv4-mapped, loopback, etc.
    FF_LOCALIP_IPV6_TYPE_SECONDARY_BIT = 0b01000000, // Temporary, duplicated, etc.
    FF_LOCALIP_IPV6_TYPE_PREFERRED_BIT = 0b10000000, // PREFER_SOURCE (manually set)
    FF_LOCALIP_IPV6_TYPE_TYPE_MASK     = 0b00011111,
    FF_LOCALIP_IPV6_TYPE_AUTO          = 0b11111111, // Used for detect option
} FFLocalIpIpv6Type;
static_assert(sizeof(FFLocalIpIpv6Type) == sizeof(uint8_t), "");

typedef struct FFLocalIpOptions
{
    FFModuleArgs moduleArgs;

    FFLocalIpType showType;
    FFLocalIpIpv6Type ipv6Type;
    FFstrbuf namePrefix;
} FFLocalIpOptions;

static_assert(sizeof(FFLocalIpOptions) <= FF_OPTION_MAX_SIZE, "FFLocalIpOptions size exceeds maximum allowed size");
