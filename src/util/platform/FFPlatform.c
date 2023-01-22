#include "FFPlatform_private.h"

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
    FFstrbuf* buffer = (FFstrbuf*) ffListAdd(dirs);
    ffStrbufInitA(buffer, 64);
    ffStrbufAppendS(buffer, path);
    ffStrbufEnsureEndsWithC(buffer, '/');
    FF_PLATFORM_PATH_UNIQUE(dirs, buffer);
}

void ffPlatformPathAddHome(FFlist* dirs, const FFPlatform* platform, const char* suffix)
{
    FFstrbuf* buffer = (FFstrbuf*) ffListAdd(dirs);
    ffStrbufInitA(buffer, 64);
    ffStrbufAppend(buffer, &platform->homeDir);
    ffStrbufAppendS(buffer, suffix);
    ffStrbufEnsureEndsWithC(buffer, '/');
    FF_PLATFORM_PATH_UNIQUE(dirs, buffer);
}

void ffPlatformPathAddEnv(FFlist* dirs, const char* env)
{
    const char* envValue = getenv(env);
    if(!ffStrSet(envValue))
        return;

    FFstrbuf value;
    ffStrbufInitA(&value, 64);
    ffStrbufAppendS(&value, envValue);

    uint32_t startIndex = 0;
    while (startIndex < value.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&value, startIndex, ':');
        value.chars[colonIndex] = '\0';

        if(!ffStrSet(value.chars + startIndex))
        {
            startIndex = colonIndex + 1;
            continue;
        }

        ffPlatformPathAddAbsolute(dirs, value.chars + startIndex);

        startIndex = colonIndex + 1;
    }

    ffStrbufDestroy(&value);
}
