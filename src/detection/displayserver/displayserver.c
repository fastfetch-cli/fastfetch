#include "displayserver.h"
#include "detection/internal.h"

bool ffdsAppendDisplay(FFDisplayServerResult* result, uint32_t width, uint32_t height, double refreshRate, uint32_t scaledWidth, uint32_t scaledHeight)
{
    if(width == 0 || height == 0)
        return false;

    FFDisplayResult* display = ffListAdd(&result->displays);
    display->width = width;
    display->height = height;
    display->refreshRate = refreshRate;
    display->scaledWidth = scaledWidth;
    display->scaledHeight = scaledHeight;

    return true;
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds, const FFinstance* instance);

const FFDisplayServerResult* ffConnectDisplayServer(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFDisplayServerResult,
        ffConnectDisplayServerImpl(&result, instance);
    );
}
