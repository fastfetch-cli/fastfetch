#include "packages.h"
#include "common/io/io.h"

#include <inttypes.h>
#include <stddef.h>

#ifdef __APPLE__
#define st_mtim st_mtimespec
#endif

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options);

const char* ffDetectPackages(FFPackagesResult* result, FFPackagesOptions* options)
{
    ffDetectPackagesImpl(result, options);

    for(uint32_t i = 0; i < offsetof(FFPackagesResult, all) / sizeof(uint32_t); ++i)
        result->all += ((uint32_t *)result)[i];

    if (result->all == 0)
        return "No packages from known package managers found";

    return NULL;
}

bool ffPackagesReadCache(FFstrbuf* cacheDir, FFstrbuf* cacheContent, const char* filePath, const char* packageId, uint32_t* result)
{
    struct stat st;
    if (stat(filePath, &st) < 0) // file doesn't exist or isn't accessable
    {
        *result = 0;
        return true;
    }

    uint64_t mtime_current = (uint64_t) st.st_mtim.tv_sec * 1000 + (uint64_t) st.st_mtim.tv_nsec / 1000000;

    ffStrbufSet(cacheDir, &instance.state.platform.cacheDir);
    ffStrbufEnsureEndsWithC(cacheDir, '/');
    ffStrbufAppendF(cacheDir, "fastfetch/packages/%s.txt", packageId);

    if (ffReadFileBuffer(cacheDir->chars, cacheContent))
    {
        uint64_t mtime_cached;
        uint32_t num_cached;
        if (sscanf(cacheContent->chars, "%" SCNu64 " %" SCNu32, &mtime_cached, &num_cached) == 2 &&
            mtime_cached == mtime_current && num_cached > 0)
        {
            *result = num_cached;
            return true;
        }
    }

    ffStrbufSetF(cacheContent, "%" PRIu64 " ", mtime_current);

    return false;
}

bool ffPackagesWriteCache(FFstrbuf* cacheDir, FFstrbuf* cacheContent, uint32_t num_elements)
{
    ffStrbufAppendF(cacheContent, "%" PRIu32, num_elements);
    return ffWriteFileBuffer(cacheDir->chars, cacheContent);
}
