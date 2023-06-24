#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/terminalshell/terminalshell.h"
#include "modules/terminal/terminal.h"
#include "util/stringUtils.h"

#include <string.h>

#define FF_TERMINAL_NUM_FORMAT_ARGS 10

void ffPrintTerminal(FFTerminalOptions* options)
{
    const FFTerminalShellResult* result = ffDetectTerminalShell();

    if(result->terminalProcessName.length == 0)
    {
        ffPrintError(FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs, "Couldn't detect terminal");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);

        if(result->terminalVersion.length)
            printf("%s %s\n", result->terminalPrettyName.chars, result->terminalVersion.chars);
        else
            ffStrbufPutTo(&result->terminalPrettyName, stdout);
    }
    else
    {
        ffPrintFormat(FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs, FF_TERMINAL_NUM_FORMAT_ARGS, (FFformatarg[]){
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
}

bool ffParseTerminalCommandOptions(FFTerminalOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_TERMINAL_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyTerminalOptions(FFTerminalOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseTerminalJsonObject(yyjson_val* module)
{
    FFTerminalOptions __attribute__((__cleanup__(ffDestroyTerminalOptions))) options;
    ffInitTerminalOptions(&options);

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

            ffPrintError(FF_TERMINAL_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintTerminal(&options);
}
