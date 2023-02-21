#pragma once

#ifndef FF_INCLUDED_detection_gamepad_gamepad
#define FF_INCLUDED_detection_gamepad_gamepad

#include "fastfetch.h"

typedef struct FFGamepadDevice
{
    FFstrbuf identifier;
    FFstrbuf name;
} FFGamepadDevice;

const char* ffDetectGamepad(const FFinstance* instance, FFlist* devices /* List of FFGamepadDevice */);

#endif
