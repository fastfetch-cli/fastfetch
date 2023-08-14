#include "packages.h"
#include "detection/internal.h"

#include <stddef.h>

void ffDetectPackagesImpl(FFPackagesResult* result);

const char* ffDetectPackages(FFPackagesResult* result)
{
    ffDetectPackagesImpl(result);

    for(uint32_t i = 0; i < offsetof(FFPackagesResult, all) / sizeof(uint32_t); ++i)
        result->all += ((uint32_t *)result)[i];

    if (result->all == 0)
        return "No packages from known package managers found";

    return NULL;
}
