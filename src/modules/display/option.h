#pragma once

#include "common/option.h"

typedef enum __attribute__((__packed__)) FFDisplayCompactType
{
    FF_DISPLAY_COMPACT_TYPE_NONE = 0,
    FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT = 1 << 0,
    FF_DISPLAY_COMPACT_TYPE_SCALED_BIT = 1 << 1,
    FF_DISPLAY_COMPACT_TYPE_REFRESH_RATE_BIT = 1 << 2,
    FF_DISPLAY_COMPACT_TYPE_UNSIGNED = UINT8_MAX,
} FFDisplayCompactType;

typedef enum __attribute__((__packed__)) FFDisplayOrder
{
    FF_DISPLAY_ORDER_NONE,
    FF_DISPLAY_ORDER_ASC,
    FF_DISPLAY_ORDER_DESC,
} FFDisplayOrder;

typedef struct FFDisplayOptions
{
    FFModuleArgs moduleArgs;

    FFDisplayCompactType compactType;
    bool preciseRefreshRate;
    FFDisplayOrder order;
} FFDisplayOptions;

static_assert(sizeof(FFDisplayOptions) <= FF_OPTION_MAX_SIZE, "FFDisplayOptions size exceeds maximum allowed size");
