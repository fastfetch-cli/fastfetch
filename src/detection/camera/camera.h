#pragma once

#include "fastfetch.h"

typedef struct FFCameraResult
{
    FFstrbuf name;
    FFstrbuf vendor;
    FFstrbuf id;
    uint32_t width;
    uint32_t height;
} FFCameraResult;

const char* ffDetectCamera(FFlist* result /* list of FFCameraResult */);
