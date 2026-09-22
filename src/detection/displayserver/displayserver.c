#include "displayserver.h"
#include "common/FFcache.h"

FFDisplayResult* ffdsAppendDisplay(
    FFDisplayServerResult* result,
    uint32_t width,
    uint32_t height,
    double refreshRate,
    uint32_t dpi,
    uint32_t preferredWidth,
    uint32_t preferredHeight,
    double preferredRefreshRate,
    uint32_t rotation,
    FFstrbuf* name,
    FFDisplayType type,
    bool primary,
    uint64_t id,
    uint32_t physicalWidth,
    uint32_t physicalHeight,
    const char* platformApi) {
    if (width == 0 || height == 0) {
        return nullptr;
    }

    FFDisplayResult* display = FF_LIST_ADD(FFDisplayResult, result->displays);
    display->width = width;
    display->height = height;
    display->refreshRate = refreshRate;
    display->dpi = dpi ?: 96; // 0 means unknown
    display->preferredWidth = preferredWidth;
    display->preferredHeight = preferredHeight;
    display->preferredRefreshRate = preferredRefreshRate;
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
    ffStrbufInit(&display->serial);
    display->drrStatus = FF_DISPLAY_DRR_STATUS_UNKNOWN;

    return display;
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds);

static FFDisplayServerResult result;

static void initDisplayServerResult(void* storage) {
    FFDisplayServerResult* ds = storage;

    ffStrbufInit(&ds->wmProcessName);
    ffStrbufInit(&ds->wmPrettyName);
    ffStrbufInit(&ds->wmProtocolName);
    ffStrbufInit(&ds->deProcessName);
    ffStrbufInit(&ds->dePrettyName);
    ffListInit(&ds->displays);

    ffConnectDisplayServerImpl(ds);
}

static void destroyDisplayServerResult(void* storage) {
    FFDisplayServerResult* ds = storage;

    ffStrbufDestroy(&ds->wmProcessName);
    ffStrbufDestroy(&ds->wmPrettyName);
    ffStrbufDestroy(&ds->wmProtocolName);
    ffStrbufDestroy(&ds->deProcessName);
    ffStrbufDestroy(&ds->dePrettyName);

    // Every display owns its name and serial, which are lost if the list is dropped as a whole
    FF_LIST_FOR_EACH (FFDisplayResult, display, ds->displays) {
        ffStrbufDestroy(&display->name);
        ffStrbufDestroy(&display->serial);
    }
    ffListDestroy(&ds->displays);
}

static FFcacheEntry ffCacheEntryDisplayServer = {
    .name = "displayServer",
    .storage = &result,
    .init = initDisplayServerResult,
    .destroy = destroyDisplayServerResult,
};

const FFDisplayServerResult* ffConnectDisplayServer() {
    return ffCacheGet(&ffCacheEntryDisplayServer);
}
