#pragma once

#include "fastfetch.h"

typedef struct FFTerminalShellResult
{
    FFstrbuf shellProcessName;
    FFstrbuf shellExe;
    const char* shellExeName; //pointer to a char in shellExe
    FFstrbuf shellPrettyName;
    FFstrbuf shellVersion;
    uint32_t shellPid;

    FFstrbuf terminalProcessName;
    FFstrbuf terminalExe;
    FFstrbuf terminalPrettyName;
    const char* terminalExeName; //pointer to a char in terminalExe
    FFstrbuf terminalVersion;
    uint32_t terminalPid;
} FFTerminalShellResult;

const FFTerminalShellResult* ffDetectTerminalShell();
