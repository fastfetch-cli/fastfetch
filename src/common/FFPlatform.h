#pragma once

#include "common/FFstrbuf.h"
#include "common/FFlist.h"

typedef struct FFPlatformSysinfo {
    FFstrbuf name;
    FFstrbuf release;
    FFstrbuf version;
    FFstrbuf architecture;
    // The base-2 logarithm of the memory page size, i.e. a page is `1 << pageSizeShift` bytes.
    // Page sizes are powers of two on every supported platform. Storing the exponent instead of
    // the byte count keeps page count arithmetic in 64 bits and makes the invariant explicit.
    uint32_t pageSizeShift;
} FFPlatformSysinfo;

typedef struct FFPlatform {
    FFstrbuf homeDir;  // Trailing slash included
    FFstrbuf cacheDir; // Trailing slash included
    FFlist configDirs; // List of FFstrbuf, trailing slash included
    FFlist dataDirs;   // List of FFstrbuf, trailing slash included
    FFstrbuf exePath;  // The real path of current exe (empty if unavailable)
    FFstrbuf cwd;      // Trailing slash included

    uint32_t pid;
#ifndef _WIN32
    uint32_t uid;
#else
    FFstrbuf sid;
#endif
    FFstrbuf userName;
    FFstrbuf fullUserName;
    FFstrbuf hostName;
    FFstrbuf userShell;

    FFPlatformSysinfo sysinfo;

#if _WIN32
    uint32_t initCP; // The code page used by the console when the program started
#endif
} FFPlatform;

void ffPlatformInit(FFPlatform* platform);
void ffPlatformDestroy(FFPlatform* platform);
