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
    #ifndef _WIN32
    struct stat st;
    if (stat(filePath, &st) < 0) // file doesn't exist or isn't accessible
    {
        *result = 0;
        return true;
    }

    if (__builtin_expect(st.st_mtim.tv_sec <= 0, false))
        return false;

    uint64_t mtime_current = (uint64_t) st.st_mtim.tv_sec * 1000ull + (uint64_t) st.st_mtim.tv_nsec / 1000000ull;
    #else
    FF_AUTO_CLOSE_FD HANDLE handle = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) // file doesn't exist or isn't accessible
    {
        *result = 0;
        return true;
    }

    uint64_t mtime_current;
    if (!GetFileTime(handle, NULL, NULL, (FILETIME*) &mtime_current))
        return false;

    mtime_current = (mtime_current - 116444736000000000ull) / 10000ull;

    if (__builtin_expect(mtime_current == 0, false))
        return false;
    #endif

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
    if (__builtin_expect(cacheContent->length == 0, false))
        return false;

    ffStrbufAppendF(cacheContent, "%" PRIu32, num_elements);
    return ffWriteFileBuffer(cacheDir->chars, cacheContent);
}
