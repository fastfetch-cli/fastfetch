#include "fastfetch.h"

#include <string.h>
#include <ctype.h>

#define FF_SHELL_MODULE_NAME "Shell"
#define FF_SHELL_NUM_FORMAT_ARGS 7

void ffPrintShell(FFinstance* instance)
{
    const FFTerminalShellResult* result = ffDetectTerminalShell(instance);

    if(result->shellProcessName.length == 0)
    {
        ffPrintError(instance, FF_SHELL_MODULE_NAME, 0, &instance->config.shellKey, &instance->config.shellFormat, FF_SHELL_NUM_FORMAT_ARGS, "Couldn't detect shell");
        return;
    }

    if(instance->config.shellFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_SHELL_MODULE_NAME, 0, &instance->config.shellKey);
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
        ffPrintFormatString(instance, FF_SHELL_MODULE_NAME, 0, &instance->config.shellKey, &instance->config.shellFormat, NULL, FF_SHELL_NUM_FORMAT_ARGS, (FFformatarg[]) {
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
