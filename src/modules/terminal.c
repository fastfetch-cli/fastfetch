#include "fastfetch.h"
#include "common/printing.h"
#include "detection/terminalshell.h"

#include <string.h>

#define FF_TERMINAL_MODULE_NAME "Terminal"
#define FF_TERMINAL_NUM_FORMAT_ARGS 10

void ffPrintTerminal(FFinstance* instance)
{
    const FFTerminalShellResult* result = ffDetectTerminalShell(instance);

    if(result->terminalProcessName.length == 0)
    {
        ffPrintError(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminal, "Couldn't detect terminal");
        return;
    }

    if(instance->config.terminal.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminal.key);

        if(strncmp(result->terminalExeName, result->terminalProcessName.chars, result->terminalProcessName.length) == 0) // if exeName starts with processName, print it. Otherwise print processName
            puts(result->terminalExeName);
        else
            ffStrbufPutTo(&result->terminalProcessName, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminal, FF_TERMINAL_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->terminalProcessName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->terminalExe},
            {FF_FORMAT_ARG_TYPE_STRING, result->terminalExeName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->shellProcessName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->shellExe},
            {FF_FORMAT_ARG_TYPE_STRING, result->shellExeName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->shellVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->userShellExe},
            {FF_FORMAT_ARG_TYPE_STRING, result->userShellExeName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->userShellVersion}
        });
    }
}
