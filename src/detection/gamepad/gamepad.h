#pragma once

#include "fastfetch.h"

typedef struct FFGamepadDevice
{
    FFstrbuf identifier;
    FFstrbuf name;
    uint8_t battery; // 0-100%
} FFGamepadDevice;

const char* ffDetectGamepad(FFlist* devices /* List of FFGamepadDevice */);
