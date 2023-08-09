#include "fastfetch.h"

typedef struct FFPhycialDisplayResult
{
    FFstrbuf name;
    uint32_t width; // native / maximum resolution, in pixels
    uint32_t height; // native / maximum resolution, in pixels
    uint32_t phycialWidth; // in mm
    uint32_t phycialHeight; // in mm
} FFPhycialDisplayResult;

const char* ffDetectPhycialDisplay(FFlist* results);
