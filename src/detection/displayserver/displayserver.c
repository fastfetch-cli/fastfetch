#include "displayserver.h"

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


static inline void ffDisplayServerResultDestory(FFDisplayServerResult* result)
{
    if (!result) return;
    ffStrbufDestroy(&result->wmProcessName);
    ffStrbufDestroy(&result->wmPrettyName);
    ffStrbufDestroy(&result->wmProtocolName);
    ffStrbufDestroy(&result->deProcessName);
    ffStrbufDestroy(&result->dePrettyName);
    FF_LIST_FOR_EACH(FFDisplayResult, display, result->displays)
    {
        ffStrbufDestroy(&display->name);
    }
    ffListDestroy(&result->displays);
}

static FFDisplayServerResult result;

void ffDisplayServerCleanResult(void)
{
    ffDisplayServerResultDestory(&result);
}

const FFDisplayServerResult* ffConnectDisplayServer()
{
    if (result.displays.elementSize == 0)
    {
        ffStrbufInit(&result.wmProcessName);
        ffStrbufInit(&result.wmPrettyName);
        ffStrbufInit(&result.wmProtocolName);
        ffStrbufInit(&result.deProcessName);
        ffStrbufInit(&result.dePrettyName);
        ffListInit(&result.displays, sizeof(FFDisplayResult));
        ffConnectDisplayServerImpl(&result);
        atexit(ffDisplayServerCleanResult);
    }
    return &result;
}
