#include "displayserver.h"

FFDisplayResult* ffdsAppendDisplay(
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
    uint64_t id,
    uint32_t physicalWidth,
    uint32_t physicalHeight,
    const char* platformApi)
{
    if(width == 0 || height == 0)
        return NULL;

    FFDisplayResult* display = ffListAdd(&result->displays);
    display->width = width;
    display->height = height;
    display->refreshRate = refreshRate;
    display->scaledWidth = scaledWidth;
    display->scaledHeight = scaledHeight;
    display->rotation = rotation;
    ffStrbufInitMove(&display->name, name);
    display->type = type;
    display->id = id;
    display->physicalWidth = physicalWidth;
    display->physicalHeight = physicalHeight;
    display->primary = primary;
    display->platformApi = platformApi;

    display->bitDepth = 0;
    display->hdrStatus = FF_DISPLAY_HDR_STATUS_UNKNOWN;
    display->manufactureYear = 0;
    display->manufactureWeek = 0;
    display->serial = 0;

    return display;
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds);

const FFDisplayServerResult* ffConnectDisplayServer()
{
    static FFDisplayServerResult result;
    if (result.displays.elementSize == 0)
    {
        ffStrbufInit(&result.wmProcessName);
        ffStrbufInit(&result.wmPrettyName);
        ffStrbufInit(&result.wmProtocolName);
        ffStrbufInit(&result.deProcessName);
        ffStrbufInit(&result.dePrettyName);
        ffListInit(&result.displays, sizeof(FFDisplayResult));
        ffConnectDisplayServerImpl(&result);
    }
    return &result;
}
