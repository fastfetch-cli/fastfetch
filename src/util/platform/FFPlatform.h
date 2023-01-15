#pragma once

#ifndef FF_INCLUDED_util_platform_FFPlatform
#define FF_INCLUDED_util_platform_FFPlatform

#include "util/FFstrbuf.h"
#include "util/FFlist.h"

typedef struct FFPlatform {
    FFstrbuf homeDir;  // Trailing slash included
    FFstrbuf cacheDir; // Trailing slash included
    FFlist configDirs; // List of FFstrbuf, trailing slash included
    FFlist dataDirs;   // List of FFstrbuf, trailing slash included

    FFstrbuf userName;
    FFstrbuf hostName;
    FFstrbuf domainName;
    FFstrbuf userShell;

    FFstrbuf systemName;
    FFstrbuf systemRelease;
    FFstrbuf systemVersion;
    FFstrbuf systemArchitecture;
} FFPlatform;

void ffPlatformInit(FFPlatform* platform);
void ffPlatformDestroy(FFPlatform* platform);

#endif
