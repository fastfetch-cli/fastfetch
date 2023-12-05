#pragma once

#include "fastfetch.h"

typedef struct FFGamepadDevice
{
    FFstrbuf identifier;
    FFstrbuf name;
} FFGamepadDevice;

const char* ffDetectGamepad(FFlist* devices /* List of FFGamepadDevice */);
