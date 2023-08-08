#include "fastfetch.h"

typedef struct FFNativeResolutionResult
{
    FFstrbuf name;
    uint32_t width;
    uint32_t height;
} FFNativeResolutionResult;

const char* ffDetectNativeResolution(FFlist* results);
