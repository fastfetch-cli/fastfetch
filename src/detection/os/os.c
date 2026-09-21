#include "os.h"
#include "common/FFcache.h"

void ffDetectOSImpl(FFOSResult* os);

static FFOSResult result;

static void initOSResult(void* storage) {
    FFOSResult* os = storage;

    ffStrbufInit(&os->name);
    ffStrbufInit(&os->prettyName);
    ffStrbufInit(&os->id);
    ffStrbufInit(&os->version);
    ffStrbufInit(&os->versionID);
    ffStrbufInit(&os->codename);
    ffStrbufInit(&os->buildID);
    ffStrbufInit(&os->idLike);
    ffStrbufInit(&os->variant);
    ffStrbufInit(&os->variantID);
    ffDetectOSImpl(os);
}

static void destroyOSResult(void* storage) {
    FFOSResult* os = storage;

    ffStrbufDestroy(&os->name);
    ffStrbufDestroy(&os->prettyName);
    ffStrbufDestroy(&os->id);
    ffStrbufDestroy(&os->version);
    ffStrbufDestroy(&os->versionID);
    ffStrbufDestroy(&os->codename);
    ffStrbufDestroy(&os->buildID);
    ffStrbufDestroy(&os->idLike);
    ffStrbufDestroy(&os->variant);
    ffStrbufDestroy(&os->variantID);
}

static FFcacheEntry ffCacheEntryOS = {
    .name = "os",
    .storage = &result,
    .init = initOSResult,
    .destroy = destroyOSResult,
};

const FFOSResult* ffDetectOS(void) {
    return ffCacheGet(&ffCacheEntryOS);
}
