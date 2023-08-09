#include "fastfetch.h"

typedef struct FFPhycialDisplayResult
{
    FFstrbuf name;
    uint32_t width;
    uint32_t height;
} FFPhycialDisplayResult;

const char* ffDetectPhycialDisplay(FFlist* results);
