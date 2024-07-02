#pragma once

#include "util/FFstrbuf.h"
#include "util/FFlist.h"

typedef struct FFPlatformSysinfo
{
    FFstrbuf name;
    FFstrbuf release;
    FFstrbuf version;
    FFstrbuf architecture;
    FFstrbuf displayVersion;
    uint32_t pageSize;
} FFPlatformSysinfo;

typedef struct FFPlatform
{
    FFstrbuf homeDir;  // Trailing slash included
    FFstrbuf cacheDir; // Trailing slash included
    FFlist configDirs; // List of FFstrbuf, trailing slash included
    FFlist dataDirs;   // List of FFstrbuf, trailing slash included
    FFstrbuf exePath; // The real path of current exe

    FFstrbuf userName;
    FFstrbuf hostName;
    FFstrbuf userShell;

    FFPlatformSysinfo sysinfo;
} FFPlatform;

void ffPlatformInit(FFPlatform* platform);
void ffPlatformDestroy(FFPlatform* platform);
