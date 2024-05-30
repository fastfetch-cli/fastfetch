#pragma once

#include "util/FFstrbuf.h"

#include <sys/types.h>

const char* ffProcessAppendOutput(FFstrbuf* buffer, char* const argv[], bool useStdErr);

static inline const char* ffProcessAppendStdOut(FFstrbuf* buffer, char* const argv[])
{
    const char* error = ffProcessAppendOutput(buffer, argv, false);
    if (!error)
        ffStrbufTrimRightSpace(buffer);
    return error;
}

static inline const char* ffProcessAppendStdErr(FFstrbuf* buffer, char* const argv[])
{
    const char* error = ffProcessAppendOutput(buffer, argv, true);
    if (!error)
        ffStrbufTrimRightSpace(buffer);
    return error;
}

#ifdef _WIN32
bool ffProcessGetInfoWindows(uint32_t pid, uint32_t* ppid, FFstrbuf* pname, FFstrbuf* exe, const char** exeName, FFstrbuf* exePath, bool* gui);
#else
void ffProcessGetInfoLinux(pid_t pid, FFstrbuf* processName, FFstrbuf* exe, const char** exeName, FFstrbuf* exePath);
const char* ffProcessGetBasicInfoLinux(pid_t pid, FFstrbuf* name, pid_t* ppid, int32_t* tty);
#endif
