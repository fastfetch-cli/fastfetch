#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"
#include "common/percent.h"

typedef enum FFSoundType
{
    FF_SOUND_TYPE_MAIN,
    FF_SOUND_TYPE_ACTIVE,
    FF_SOUND_TYPE_ALL,
} FFSoundType;

typedef struct FFSoundOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

    FFSoundType soundType;
    FFColorRangeConfig percent;
} FFSoundOptions;
