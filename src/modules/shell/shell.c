#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/terminalshell/terminalshell.h"
#include "modules/shell/shell.h"
#include "util/stringUtils.h"

#define FF_SHELL_NUM_FORMAT_ARGS 7

void ffPrintShell(FFShellOptions* options)
{
    const FFTerminalShellResult* result = ffDetectTerminalShell();

    if(result->shellProcessName.length == 0)
    {
        ffPrintError(FF_SHELL_MODULE_NAME, 0, &options->moduleArgs, "Couldn't detect shell");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_SHELL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&result->shellPrettyName, stdout);

        if(result->shellVersion.length > 0)
        {
            putchar(' ');
            ffStrbufWriteTo(&result->shellVersion, stdout);
        }

        putchar('\n');
    }
    else
    {
        ffPrintFormat(FF_SHELL_MODULE_NAME, 0, &options->moduleArgs, FF_SHELL_NUM_FORMAT_ARGS, (FFformatarg[]) {
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

void ffInitShellOptions(FFShellOptions* options)
{
    options->moduleName = FF_SHELL_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseShellCommandOptions(FFShellOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_SHELL_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyShellOptions(FFShellOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseShellJsonObject(yyjson_val* module)
{
    FFShellOptions __attribute__((__cleanup__(ffDestroyShellOptions))) options;
    ffInitShellOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(ffStrEqualsIgnCase(key, "type"))
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(FF_SHELL_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintShell(&options);
}
