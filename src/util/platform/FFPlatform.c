#include "FFPlatform_private.h"
#include "util/stringUtils.h"
#include "common/io/io.h"

void ffPlatformInit(FFPlatform* platform)
{
    ffStrbufInit(&platform->homeDir);
    ffStrbufInit(&platform->cacheDir);
    ffListInit(&platform->configDirs, sizeof(FFstrbuf));
    ffListInit(&platform->dataDirs, sizeof(FFstrbuf));

    ffStrbufInit(&platform->userName);
    ffStrbufInit(&platform->hostName);
    ffStrbufInit(&platform->domainName);
    ffStrbufInit(&platform->userShell);

    ffStrbufInit(&platform->systemName);
    ffStrbufInit(&platform->systemRelease);
    ffStrbufInit(&platform->systemVersion);
    ffStrbufInit(&platform->systemArchitecture);

    ffPlatformInitImpl(platform);

    if(platform->domainName.length == 0)
        ffStrbufAppend(&platform->domainName, &platform->hostName);

    if(platform->systemName.length == 0)
    {
        #if defined(__linux__)
            ffStrbufAppendS(&platform->systemName, "Linux");
        #elif defined(__FreeBSD__)
            ffStrbufAppendS(&platform->systemName, "FreeBSD");
        #elif defined(__APPLE__)
            ffStrbufAppendS(&platform->systemName, "Darwin");
        #elif defined(_WIN32)
            ffStrbufAppendS(&platform->systemName, "Windows_NT");
        #else
            ffStrbufAppendS(&platform->systemName, "Unknown");
        #endif
    }

    if(platform->systemArchitecture.length == 0)
        ffStrbufAppendS(&platform->systemArchitecture, "Unknown");
}

void ffPlatformDestroy(FFPlatform* platform)
{
    ffStrbufDestroy(&platform->homeDir);
    ffStrbufDestroy(&platform->cacheDir);

    FF_LIST_FOR_EACH(FFstrbuf, dir, platform->configDirs)
        ffStrbufDestroy(dir);
    ffListDestroy(&platform->configDirs);

    FF_LIST_FOR_EACH(FFstrbuf, dir, platform->dataDirs)
        ffStrbufDestroy(dir);
    ffListDestroy(&platform->dataDirs);

    ffStrbufDestroy(&platform->userName);
    ffStrbufDestroy(&platform->hostName);
    ffStrbufDestroy(&platform->domainName);
    ffStrbufDestroy(&platform->userShell);

    ffStrbufDestroy(&platform->systemArchitecture);
    ffStrbufDestroy(&platform->systemName);
    ffStrbufDestroy(&platform->systemRelease);
    ffStrbufDestroy(&platform->systemVersion);
}

void ffPlatformPathAddAbsolute(FFlist* dirs, const char* path)
{
    if (!ffPathExists(path, FF_PATHTYPE_DIRECTORY))
        return;

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreateS(path);
    ffStrbufEnsureEndsWithC(&buffer, '/');
    if (!ffListContains(dirs, &buffer, (void*) ffStrbufEqual))
        ffStrbufInitMove((FFstrbuf*) ffListAdd(dirs), &buffer);
}

void ffPlatformPathAddHome(FFlist* dirs, const FFPlatform* platform, const char* suffix)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreateA(64);
    ffStrbufAppend(&buffer, &platform->homeDir);
    ffStrbufAppendS(&buffer, suffix);
    ffStrbufEnsureEndsWithC(&buffer, '/');
    if (ffPathExists(buffer.chars, FF_PATHTYPE_DIRECTORY) && !ffListContains(dirs, &buffer, (void*) ffStrbufEqual))
        ffStrbufInitMove((FFstrbuf*) ffListAdd(dirs), &buffer);
}
