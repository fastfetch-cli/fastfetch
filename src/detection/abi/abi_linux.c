#include "abi.h"

#ifdef __loongarch__

// Detect the running kernel's ABI flavor and compatibility.
//
// Reference: https://areweloongyet.com/docs/world-compat-details/

#include <errno.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_fstat
#define __NR_fstat 80
#endif

#ifndef __NR_getrlimit
#define __NR_getrlimit 163
#endif

#define NSIG_NEW_WORLD 65
#define NSIG_BYTES_NEW_WORLD 8
#define NSIG_BYTES_OLD_WORLD 16

const char* ffDetectABI(
    FF_MAYBE_UNUSED const FFABIOptions* options,
    FFABIResult* result)
{
    // record the ABI used at build-time
    bool builtOnNewWorld = NSIG == NSIG_NEW_WORLD;

    FFABICompat* buildTimeItem = (FFABICompat*) ffListAdd(&result->compats);
    *buildTimeItem = (FFABICompat) {
        .name = FASTFETCH_PROJECT_NAME " program",
        .desc = builtOnNewWorld ? "New world (ABI2.0)" : "Old world (ABI1.0)",
    };

    // probe the running kernel
    bool hasNewWorldSignals = syscall(__NR_rt_sigaction, SIGUSR1, NULL, NULL, NSIG_BYTES_NEW_WORLD) == 0;
    bool hasOldWorldSignals = syscall(__NR_rt_sigaction, SIGUSR1, NULL, NULL, NSIG_BYTES_OLD_WORLD) == 0;

    bool hasFstatSyscalls = true;
    if (syscall(__NR_fstat, 0, NULL) == -1)
    {
        hasFstatSyscalls = errno != ENOSYS;
    }

    bool hasOldRlimitSyscalls = true;
    if (syscall(__NR_getrlimit, RLIMIT_NOFILE, NULL) == -1)
    {
        hasOldRlimitSyscalls = errno != ENOSYS;
    }

    ffABIAddFeature(result, "new-world signals", hasNewWorldSignals);
    ffABIAddFeature(result, "old-world signals", hasOldWorldSignals);
    ffABIAddFeature(result, "fstat & newfstatat", hasFstatSyscalls);
    ffABIAddFeature(result, "getrlimit & setrlimit", hasOldRlimitSyscalls);

    // now classify the system
    bool fullyOldWorld = hasOldWorldSignals && hasFstatSyscalls && hasOldRlimitSyscalls;
    bool fullyNewWorld = hasNewWorldSignals;
    bool isNewWorldPast611 = fullyNewWorld && hasFstatSyscalls;

    FFABICompat* newWorldItem = (FFABICompat*) ffListAdd(&result->compats);
    *newWorldItem = (FFABICompat) {
        .name = "LoongArch new world",
        .desc = NULL,
    };

    FFABICompat* oldWorldItem = (FFABICompat*) ffListAdd(&result->compats);
    *oldWorldItem = (FFABICompat) {
        .name = "LoongArch old world",
        .desc = NULL,
    };

    if (fullyNewWorld)
    {
        if (isNewWorldPast611)
            newWorldItem->desc = "Supported (Linux 6.11)";
        else
            newWorldItem->desc = "Supported (Linux 5.19)";

        if (fullyOldWorld)
            oldWorldItem->desc = "Supported (compatible)";
        else
            oldWorldItem->desc = "Unsupported";
    }
    else
    {
        newWorldItem->desc = "Unsupported";
        oldWorldItem->desc = "Supported (native)";
    }

    return NULL;
}

#else

const char* ffDetectABI(
    FF_MAYBE_UNUSED const FFABIOptions* options,
    FF_MAYBE_UNUSED FFABIResult* result)
{
    return "Not supported on this architecture";
}

#endif
