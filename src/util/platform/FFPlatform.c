#include "FFPlatform_private.h"
#include "util/stringUtils.h"
#include "common/io/io.h"
#include "detection/version/version.h"

void ffPlatformInit(FFPlatform* platform)
{
    ffStrbufInit(&platform->homeDir);
    ffStrbufInit(&platform->cacheDir);
    ffListInit(&platform->configDirs, sizeof(FFstrbuf));
    ffListInit(&platform->dataDirs, sizeof(FFstrbuf));
    ffStrbufInit(&platform->exePath);

    ffStrbufInit(&platform->userName);
    ffStrbufInit(&platform->hostName);
    ffStrbufInit(&platform->userShell);

    FFPlatformSysinfo* info = &platform->sysinfo;

    ffStrbufInit(&info->name);
    ffStrbufInit(&info->release);
    ffStrbufInit(&info->version);
    ffStrbufInit(&info->architecture);

    ffPlatformInitImpl(platform);

    if(info->name.length == 0)
        ffStrbufSetStatic(&info->name, ffVersionResult.sysName);

    if(info->architecture.length == 0)
        ffStrbufSetStatic(&info->architecture, ffVersionResult.architecture);
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
    ffStrbufDestroy(&platform->exePath);

    ffStrbufDestroy(&platform->userName);
    ffStrbufDestroy(&platform->hostName);
    ffStrbufDestroy(&platform->userShell);

    FFPlatformSysinfo* info = &platform->sysinfo;
    ffStrbufDestroy(&info->architecture);
    ffStrbufDestroy(&info->name);
    ffStrbufDestroy(&info->release);
    ffStrbufDestroy(&info->version);
    ffStrbufDestroy(&info->displayVersion);
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
