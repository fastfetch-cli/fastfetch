#pragma once

#include "fastfetch.h"

typedef struct FFShellResult
{
    FFstrbuf processName;
    FFstrbuf exe; //Actually arg0 in *nix
    const char* exeName; //pointer to a char in exe
    FFstrbuf exePath; //Full real path to executable file
    FFstrbuf prettyName;
    FFstrbuf version;
    uint32_t pid;
    uint32_t ppid;
    int32_t tty;
} FFShellResult;

static inline void ffShellResultDestory(FFShellResult *result)
{
    if (!result) return;
    ffStrbufDestroy(&result->processName);
    result->exeName = NULL;
    ffStrbufDestroy(&result->exe);
    ffStrbufDestroy(&result->exePath);
    ffStrbufDestroy(&result->prettyName);
    ffStrbufDestroy(&result->version);
}

typedef struct FFTerminalResult
{
    FFstrbuf processName;
    FFstrbuf exe;
    FFstrbuf prettyName;
    const char* exeName; //pointer to a char in exe
    FFstrbuf exePath; //Full real path to executable file
    FFstrbuf version;
    FFstrbuf tty;
    uint32_t pid;
    uint32_t ppid;
} FFTerminalResult;

static inline void ffTerminalResultDestory(FFTerminalResult *result)
{
    if (!result) return;
    ffStrbufDestroy(&result->processName);
    result->exeName = NULL;
    ffStrbufDestroy(&result->exe);
    ffStrbufDestroy(&result->prettyName);
    ffStrbufDestroy(&result->exePath);
    ffStrbufDestroy(&result->version);
    ffStrbufDestroy(&result->tty);
}

const FFShellResult* ffDetectShell();
const FFTerminalResult* ffDetectTerminal();
