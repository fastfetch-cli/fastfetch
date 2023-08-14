#pragma once

#ifndef FF_INCLUDED_detection_terminalshell
#define FF_INCLUDED_detection_terminalshell

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

    FFstrbuf userShellExe;
    const char* userShellExeName; //pointer to a char in userShellExe
    FFstrbuf userShellVersion;
} FFTerminalShellResult;

const FFTerminalShellResult* ffDetectTerminalShell();

#endif
