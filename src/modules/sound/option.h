#pragma once

#include "common/option.h"
#include "common/percent.h"

typedef enum FF_A_PACKED FFSoundType {
    FF_SOUND_TYPE_NONE = 0,
    FF_SOUND_TYPE_MAIN = 1 << 0,
    FF_SOUND_TYPE_ACTIVE = 1 << 1,
} FFSoundType;

typedef struct FFSoundOptions {
    FFModuleArgs moduleArgs;

    // Reports matched device only, otherwise reports all devices
    // NOTE: for FF_SOUND_TYPE_NONE, reports all devices
    FFSoundType soundType;
    FFPercentageModuleConfig percent;
} FFSoundOptions;

static_assert(sizeof(FFSoundOptions) <= FF_OPTION_MAX_SIZE, "FFSoundOptions size exceeds maximum allowed size");
