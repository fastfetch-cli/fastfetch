#include "fastfetch.h"

typedef struct FFPhysicalDisplayResult
{
    FFstrbuf name;
    uint32_t width; // native / maximum resolution, in pixels
    uint32_t height; // native / maximum resolution, in pixels
    uint32_t physicalWidth; // in mm
    uint32_t physicalHeight; // in mm
} FFPhysicalDisplayResult;

const char* ffDetectPhysicalDisplay(FFlist* results);
