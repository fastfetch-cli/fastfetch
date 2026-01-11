#pragma once

#include "common/FFstrbuf.h"

#ifndef _WIN32
#include <sys/types.h> // pid_t
#endif

typedef struct FFProcessHandle {
    #if _WIN32
    void* pid; // HANDLE
    void* pipeRead; // HANDLE
    #else
    pid_t pid;
    int pipeRead;
    #endif
} FFProcessHandle;

const char* ffProcessSpawn(char* const argv[], bool useStdErr, FFProcessHandle* outHandle);
const char* ffProcessReadOutput(FFProcessHandle* handle, FFstrbuf* buffer); // Destroys handle internally

static inline const char* ffProcessAppendStdOut(FFstrbuf* buffer, char* const argv[])
{
    FFProcessHandle handle;
    const char* error = ffProcessSpawn(argv, false, &handle);
    if (error) return error;

    error = ffProcessReadOutput(&handle, buffer);
    if (!error)
        ffStrbufTrimRightSpace(buffer);
    return error;
}

static inline const char* ffProcessAppendStdErr(FFstrbuf* buffer, char* const argv[])
{
    FFProcessHandle handle;
    const char* error = ffProcessSpawn(argv, true, &handle);
    if (error) return error;

    error = ffProcessReadOutput(&handle, buffer);
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
