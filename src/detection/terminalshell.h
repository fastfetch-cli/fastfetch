#pragma once

#ifndef FF_INCLUDED_detection_terminalshell
#define FF_INCLUDED_detection_terminalshell

#include "fastfetch.h"

typedef struct FFTerminalShellResult
{
    FFstrbuf shellProcessName;
    FFstrbuf shellExe;
    const char* shellExeName; //pointer to a char in shellExe
    FFstrbuf shellVersion;

    FFstrbuf terminalProcessName;
    FFstrbuf terminalExe;
    const char* terminalExeName; //pointer to a char in terminalExe

    FFstrbuf userShellExe;
    const char* userShellExeName; //pointer to a char in userShellExe
    FFstrbuf userShellVersion;
} FFTerminalShellResult;

const FFTerminalShellResult* ffDetectTerminalShell(const FFinstance* instance);

#endif
