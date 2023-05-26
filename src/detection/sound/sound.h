#pragma once

#ifndef FF_INCLUDED_detection_sound_sound
#define FF_INCLUDED_detection_sound_sound

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

const char* ffDetectSound(const FFinstance* instance, FFlist* devices /* List of FFSoundDevice */);

#endif
