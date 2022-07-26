#include "fastfetch.h"
#include "common/printing.h"
#include "detection/terminalshell.h"

#define FF_SHELL_MODULE_NAME "Shell"
#define FF_SHELL_NUM_FORMAT_ARGS 7

void ffPrintShell(FFinstance* instance)
{
    const FFTerminalShellResult* result = ffDetectTerminalShell(instance);

    if(result->shellProcessName.length == 0)
    {
        ffPrintError(instance, FF_SHELL_MODULE_NAME, 0, &instance->config.shell, "Couldn't detect shell");
        return;
    }

    if(instance->config.shell.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_SHELL_MODULE_NAME, 0, &instance->config.shell.key);
        fputs(result->shellExeName, stdout);

        if(result->shellVersion.length > 0)
        {
            putchar(' ');
            ffStrbufWriteTo(&result->shellVersion, stdout);
        }

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_SHELL_MODULE_NAME, 0, &instance->config.shell, FF_SHELL_NUM_FORMAT_ARGS, (FFformatarg[]) {
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
