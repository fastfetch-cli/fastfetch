#include "fastfetch.h"
#include "common/printing.h"
#include "detection/terminalshell/terminalshell.h"
#include "modules/shell/shell.h"

#define FF_SHELL_NUM_FORMAT_ARGS 7

void ffPrintShell(FFinstance* instance, FFShellOptions* options)
{
    const FFTerminalShellResult* result = ffDetectTerminalShell(instance);

    if(result->shellProcessName.length == 0)
    {
        ffPrintError(instance, FF_SHELL_MODULE_NAME, 0, &options->moduleArgs, "Couldn't detect shell");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_SHELL_MODULE_NAME, 0, &options->moduleArgs.key);
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
        ffPrintFormat(instance, FF_SHELL_MODULE_NAME, 0, &options->moduleArgs, FF_SHELL_NUM_FORMAT_ARGS, (FFformatarg[]) {
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

    options->version = true;
}

bool ffParseShellCommandOptions(FFShellOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_SHELL_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (strcasecmp(subKey, "version") == 0)
    {
        options->version = ffOptionParseBoolean(value);
        return true;
    }

    return false;
}

void ffDestroyShellOptions(FFShellOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

#ifdef FF_HAVE_JSONC
void ffParseShellJsonObject(FFinstance* instance, json_object* module)
{
    FFShellOptions __attribute__((__cleanup__(ffDestroyShellOptions))) options;
    ffInitShellOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            if (strcasecmp(key, "version") == 0)
            {
                options.version = (bool) json_object_get_boolean(val);
                continue;
            }

            ffPrintError(instance, FF_SHELL_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintShell(instance, &options);
}
#endif
