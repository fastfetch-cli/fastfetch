#pragma once

#include "fastfetch.h"

#define FF_SOUND_VOLUME_UNKNOWN 255

typedef struct FFSoundDevice
{
    FFstrbuf identifier;
    FFstrbuf name;
    uint8_t volume; // 0-100%
    bool main;
    bool active;
} FFSoundDevice;

const char* ffDetectSound(FFlist* devices /* List of FFSoundDevice */);
