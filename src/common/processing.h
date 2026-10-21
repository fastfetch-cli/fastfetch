#pragma once

#include "common/FFstrbuf.h"
#include "common/io.h" // FFNativeFD, ffGetNullFD

#ifndef _WIN32
    #include <sys/types.h> // pid_t
#endif

typedef struct FFProcessHandle {
#if _WIN32
    void* pid;      // HANDLE
    void* pipeRead; // HANDLE
#else
    pid_t pid;
    int pipeRead;
#endif
} FFProcessHandle;

// Which of the child's two output streams are wired to the pipe that `ffProcessReadOutput()` drains.
// The stream that is not selected is pointed at the null device, so it can neither interleave with
// the captured text nor fill the pipe and block the child. This is a bitfield rather than a value
// because `BOTH` is a combination: each backend sets up the child's two stdio slots independently,
// testing one bit each.
typedef enum FFProcessOutputType: uint8_t {
    FF_PROCESS_OUTPUT_STDOUT_BIT = 1 << 0,
    FF_PROCESS_OUTPUT_STDERR_BIT = 1 << 1,
    FF_PROCESS_OUTPUT_BOTH = FF_PROCESS_OUTPUT_STDOUT_BIT | FF_PROCESS_OUTPUT_STDERR_BIT,
} FFProcessOutputType;

// `stdinFd` is the fd the child process gets as its stdin. Pass `ffGetNullFD()` to detach it from
// our stdin, or `FF_PROCESS_INHERIT_STDIN` to inherit ours, which is what the helpers below do.
// Detaching matters on Android, where `/system/bin/cmd` forwards its stdin over binder to the
// service: the kernel rejects the whole transaction when that fd is a terminal, and the tool then
// only reports `Failure calling service <name>: Failed transaction`.
#define FF_PROCESS_INHERIT_STDIN ((FFNativeFD) -1)
const char* ffProcessSpawn(char* const argv[], FFProcessOutputType useOutput, FFNativeFD stdinFd, FFProcessHandle* outHandle);
const char* ffProcessReadOutput(FFProcessHandle* handle, FFstrbuf* buffer); // Destroys handle internally

static inline const char* ffProcessAppendStdOut(FFstrbuf* buffer, char* const argv[]) {
    FFProcessHandle handle;
    const char* error = ffProcessSpawn(argv, FF_PROCESS_OUTPUT_STDOUT_BIT, FF_PROCESS_INHERIT_STDIN, &handle);
    if (error) {
        return error;
    }

    error = ffProcessReadOutput(&handle, buffer);
    if (!error) {
        ffStrbufTrimRightSpace(buffer);
    }
    return error;
}

static inline const char* ffProcessAppendStdErr(FFstrbuf* buffer, char* const argv[]) {
    FFProcessHandle handle;
    const char* error = ffProcessSpawn(argv, FF_PROCESS_OUTPUT_STDERR_BIT, FF_PROCESS_INHERIT_STDIN, &handle);
    if (error) {
        return error;
    }

    error = ffProcessReadOutput(&handle, buffer);
    if (!error) {
        ffStrbufTrimRightSpace(buffer);
    }
    return error;
}

#ifdef _WIN32
bool ffProcessGetInfoWindows(uint32_t pid, uint32_t* ppid, FFstrbuf* pname, FFstrbuf* exe, const char** exeName, FFstrbuf* exePath, bool* gui);
#else
void ffProcessGetInfoLinux(pid_t pid, FFstrbuf* processName, FFstrbuf* exe, const char** exeName, FFstrbuf* exePath);
const char* ffProcessGetBasicInfoLinux(pid_t pid, FFstrbuf* name, pid_t* ppid, int32_t* tty);
#endif
