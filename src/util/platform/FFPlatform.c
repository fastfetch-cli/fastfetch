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

    ffPlatformInitÎmpl(platform);

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
        #elif defined(__WIN32__) || defined(__WIN64__)
            ffStrbufAppendS(&platform->systemName, "Windows_NT");
        #endif
    }

    if(platform->systemArchitecture.length == 0)
    {
        #if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64)
            ffStrbufAppendS(&platform->systemArchitecture, "x86_64");
        #elif defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(__i586__) || defined(__i586) || defined(__i686__) || defined(__i686)
            ffStrbufAppendS(&platform->systemArchitecture, "i386");
        #elif defined(__aarch64__)
            ffStrbufAppendS(&platform->systemArchitecture, "aarch64");
        #elif defined(__arm__)
            ffStrbufAppendS(&platform->systemArchitecture, "arm");
        #endif
    }
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
