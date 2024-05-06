#include "os.h"

void ffDetectOSImpl(FFOSResult* os);

static inline void ffOSResultDestory(FFOSResult* result)
{
    if (!result) return;
    ffStrbufDestroy(&result->name);
    ffStrbufDestroy(&result->prettyName);
    ffStrbufDestroy(&result->id);
    ffStrbufDestroy(&result->idLike);
    ffStrbufDestroy(&result->variant);
    ffStrbufDestroy(&result->variantID);
    ffStrbufDestroy(&result->version);
    ffStrbufDestroy(&result->versionID);
    ffStrbufDestroy(&result->codename);
    ffStrbufDestroy(&result->buildID);
}

static FFOSResult result;

static void cleanupResult(void)
{
    ffOSResultDestory(&result);
}

const FFOSResult* ffDetectOS(void)
{
    if (result.name.chars == NULL)
    {
        ffStrbufInit(&result.name);
        ffStrbufInit(&result.prettyName);
        ffStrbufInit(&result.id);
        ffStrbufInit(&result.version);
        ffStrbufInit(&result.versionID);
        ffStrbufInit(&result.codename);
        ffStrbufInit(&result.buildID);
        ffStrbufInit(&result.idLike);
        ffStrbufInit(&result.variant);
        ffStrbufInit(&result.variantID);
        ffDetectOSImpl(&result);
        atexit(cleanupResult);
    }
    return &result;
}
