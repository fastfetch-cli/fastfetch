#include "fastfetch.h"

#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define FF_TERMINAL_MODULE_NAME "Terminal"
#define FF_TERMINAL_NUM_FORMAT_ARGS 3

void ffPrintTerminal(FFinstance* instance)
{
    const FFTerminalShellResult* result = ffDetectTerminalShell(instance);

    if(result->terminalProcessName.length == 0)
    {
        ffPrintError(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey, &instance->config.terminalFormat, FF_TERMINAL_NUM_FORMAT_ARGS, "Couldn't detect terminal");
        return;
    }

    if(instance->config.terminalFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey);

        uint32_t terminalExeNameLength = result->terminalExe.chars + result->terminalExe.length - result->terminalExeName;
        if(strncmp(result->terminalExeName, result->terminalProcessName.chars, terminalExeNameLength) == 0) // if exeName starts with processName, print it. Otherwise print processName
            puts(result->terminalExeName);
        else
            ffStrbufPutTo(&result->terminalProcessName, stdout);
    }
    else
    {
        ffPrintFormatString(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey, &instance->config.terminalFormat, NULL, FF_TERMINAL_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->terminalProcessName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->terminalExe},
            {FF_FORMAT_ARG_TYPE_STRING, result->terminalExeName}
        });
    }
}
