#pragma once

#include "common/option.h"
#include "common/percent.h"

typedef enum __attribute__((__packed__)) FFSoundType
{
    FF_SOUND_TYPE_MAIN,
    FF_SOUND_TYPE_ACTIVE,
    FF_SOUND_TYPE_ALL,
} FFSoundType;

typedef struct FFSoundOptions
{
    FFModuleArgs moduleArgs;

    FFSoundType soundType;
    FFPercentageModuleConfig percent;
} FFSoundOptions;

static_assert(sizeof(FFSoundOptions) <= FF_OPTION_MAX_SIZE, "FFSoundOptions size exceeds maximum allowed size");
