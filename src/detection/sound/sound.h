#pragma once

#ifndef FF_INCLUDED_detection_sound_sound
#define FF_INCLUDED_detection_sound_sound

#include "fastfetch.h"

typedef struct FFSoundDevice
{
    FFstrbuf identifier; // Used internally, not guaranteed to be initialized
    FFstrbuf name;
    FFstrbuf manufacturer;
    uint8_t volume; // 0-100%
    bool main;
    bool active;
} FFSoundDevice;

const char* ffDetectSound(const FFinstance* instance, FFlist* devices /* List of FFSoundDevice */);

#endif
