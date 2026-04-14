#pragma once

#include "fastfetch.h"
#include "modules/sound/option.h"

#define FF_SOUND_VOLUME_UNKNOWN 255

typedef struct FFSoundDevice {
    FFstrbuf identifier;
    FFstrbuf name;
    FFstrbuf platformApi;
    uint8_t volume; // 0-100%
    FFSoundType type;
} FFSoundDevice;

const char* ffDetectSound(FFSoundOptions* options, FFlist* devices /* List of FFSoundDevice */);
