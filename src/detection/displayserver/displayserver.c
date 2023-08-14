#include "displayserver.h"
#include "detection/internal.h"

bool ffdsAppendDisplay(
    FFDisplayServerResult* result,
    uint32_t width,
    uint32_t height,
    double refreshRate,
    uint32_t scaledWidth,
    uint32_t scaledHeight,
    uint32_t rotation,
    FFstrbuf* name,
    FFDisplayType type,
    bool primary,
    uint64_t id)
{
    if(width == 0 || height == 0)
        return false;

    FFDisplayResult* display = ffListAdd(&result->displays);
    display->width = width;
    display->height = height;
    display->refreshRate = refreshRate;
    display->scaledWidth = scaledWidth;
    display->scaledHeight = scaledHeight;
    display->rotation = rotation;
    ffStrbufInitMove(&display->name, name);
    display->type = type;
    display->primary = primary;
    display->id = id;

    return true;
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds);

const FFDisplayServerResult* ffConnectDisplayServer()
{
    FF_DETECTION_INTERNAL_GUARD(FFDisplayServerResult,
        ffConnectDisplayServerImpl(&result);
    );
}
