#pragma once

#include "fastfetch.h"

typedef struct FFShellResult
{
    FFstrbuf processName;
    FFstrbuf exe; //Actually arg0
    const char* exeName; //pointer to a char in exe
    FFstrbuf exePath; //Full real path to executable file
    FFstrbuf prettyName;
    FFstrbuf version;
    uint32_t pid;
    uint32_t ppid;
} FFShellResult;

typedef struct FFTerminalResult
{
    FFstrbuf processName;
    FFstrbuf exe;
    FFstrbuf prettyName;
    const char* exeName; //pointer to a char in exe
    FFstrbuf exePath; //Full real path to executable file
    FFstrbuf version;
    uint32_t pid;
    uint32_t ppid;
} FFTerminalResult;

const FFShellResult* ffDetectShell();
const FFTerminalResult* ffDetectTerminal();
