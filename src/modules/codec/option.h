#pragma once

#include "common/option.h"

typedef enum FF_A_PACKED FFCodecShowType {
    FF_CODEC_SHOW_TYPE_NONE = 0,
    FF_CODEC_SHOW_TYPE_ENCODER = 1 << 0,
    FF_CODEC_SHOW_TYPE_DECODER = 1 << 1,
    FF_CODEC_SHOW_TYPE_BOTH = FF_CODEC_SHOW_TYPE_ENCODER | FF_CODEC_SHOW_TYPE_DECODER,
    FF_CODEC_SHOW_TYPE_FORCE_UNSIGNED = UINT8_MAX,
} FFCodecShowType;

typedef struct FFCodecOptions {
    FFModuleArgs moduleArgs;
    bool splitGPU;
    bool useVulkan;
    FFCodecShowType showType;
} FFCodecOptions;

static_assert(sizeof(FFCodecOptions) <= FF_OPTION_MAX_SIZE, "FFCodecOptions size exceeds maximum allowed size");
