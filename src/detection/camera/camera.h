#pragma once

#include "fastfetch.h"
#include "modules/camera/option.h"

typedef struct FFCameraResult
{
    FFstrbuf name;
    FFstrbuf vendor;
    FFstrbuf id;
    FFstrbuf colorspace;
    uint32_t width;
    uint32_t height;
} FFCameraResult;

const char* ffDetectCamera(FFlist* result /* list of FFCameraResult */);
