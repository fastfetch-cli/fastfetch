#include "fastfetch.h"
#include "common/printing.h"
#include "detection/terminalshell/terminalshell.h"
#include "modules/terminal/terminal.h"

#include <string.h>

#define FF_TERMINAL_NUM_FORMAT_ARGS 10

void ffPrintTerminal(FFinstance* instance, FFTerminalOptions* options)
{
    const FFTerminalShellResult* result = ffDetectTerminalShell(instance);

    if(result->terminalProcessName.length == 0)
    {
        ffPrintError(instance, FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs, "Couldn't detect terminal");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs.key);

        if(result->terminalVersion.length)
            printf("%s %s\n", result->terminalPrettyName.chars, result->terminalVersion.chars);
        else
            ffStrbufPutTo(&result->terminalPrettyName, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs, FF_TERMINAL_NUM_FORMAT_ARGS, (FFformatarg[]){
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

void ffInitTerminalOptions(FFTerminalOptions* options)
{
    options->moduleName = FF_TERMINAL_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);

    options->version = true;
}

bool ffParseTerminalCommandOptions(FFTerminalOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_TERMINAL_MODULE_NAME);
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

void ffDestroyTerminalOptions(FFTerminalOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

#ifdef FF_HAVE_JSONC
void ffParseTerminalJsonObject(FFinstance* instance, json_object* module)
{
    FFTerminalOptions __attribute__((__cleanup__(ffDestroyTerminalOptions))) options;
    ffInitTerminalOptions(&options);

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

            ffPrintError(instance, FF_TERMINAL_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintTerminal(instance, &options);
}
#endif
