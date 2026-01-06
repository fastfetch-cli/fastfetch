#pragma once

#include "common/option.h"

typedef enum __attribute__((__packed__)) FFDNSShowType {
    FF_DNS_TYPE_IPV4_BIT = 1,
    FF_DNS_TYPE_IPV6_BIT = 2,
    FF_DNS_TYPE_BOTH = FF_DNS_TYPE_IPV4_BIT | FF_DNS_TYPE_IPV6_BIT,
    FF_DNS_TYPE_FORCE_UNSIGNED = UINT8_MAX,
} FFDNSShowType;

typedef struct FFDNSOptions
{
    FFModuleArgs moduleArgs;

    FFDNSShowType showType;
} FFDNSOptions;

static_assert(sizeof(FFDNSOptions) <= FF_OPTION_MAX_SIZE, "FFDNSOptions size exceeds maximum allowed size");
