#include "packages.h"
#include "detection/internal.h"

#include <stddef.h>

void ffDetectPackagesImpl(const FFinstance* instance, FFPackagesResult* result);

const FFPackagesResult* ffDetectPackages(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFPackagesResult,
        memset(&result, 0, sizeof(FFPackagesResult));
        ffStrbufInit(&result.pacmanBranch);

        ffDetectPackagesImpl(instance, &result);

        for(uint32_t i = 0; i < offsetof(FFPackagesResult, all) / sizeof(uint32_t); ++i)
            result.all += ((uint32_t *)&result)[i];
    );
}
