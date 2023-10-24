#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/terminalshell/terminalshell.h"
#include "modules/terminal/terminal.h"
#include "util/stringUtils.h"

#include <string.h>

#define FF_TERMINAL_NUM_FORMAT_ARGS 6

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
        ffPrintLogoAndKey(FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

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
            {FF_FORMAT_ARG_TYPE_UINT, &result->terminalPid},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->terminalPrettyName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->terminalVersion},
        });
    }
}

bool ffParseTerminalCommandOptions(FFTerminalOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_TERMINAL_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseTerminalJsonObject(FFTerminalOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateTerminalJsonConfig(FFTerminalOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyTerminalOptions))) FFTerminalOptions defaultOptions;
    ffInitTerminalOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateTerminalJsonResult(FF_MAYBE_UNUSED FFTerminalOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFTerminalShellResult* result = ffDetectTerminalShell();

    if(result->terminalProcessName.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "Couldn't detect terminal");
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "processName", &result->terminalProcessName);
    yyjson_mut_obj_add_strbuf(doc, obj, "exe", &result->terminalExe);
    yyjson_mut_obj_add_strcpy(doc, obj, "exeName", result->terminalExeName);
    yyjson_mut_obj_add_uint(doc, obj, "pid", result->terminalPid);
    yyjson_mut_obj_add_strbuf(doc, obj, "prettyName", &result->terminalPrettyName);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result->terminalVersion);
}

void ffPrintTerminalHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_TERMINAL_MODULE_NAME, "{5} {6}", FF_TERMINAL_NUM_FORMAT_ARGS, (const char* []) {
        "Terminal process name",
        "Terminal path with exe name",
        "Terminal exe name",
        "Terminal pid",
        "Terminal pretty name",
        "Terminal version"
    });
}

void ffInitTerminalOptions(FFTerminalOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_TERMINAL_MODULE_NAME,
        ffParseTerminalCommandOptions,
        ffParseTerminalJsonObject,
        ffPrintTerminal,
        ffGenerateTerminalJsonResult,
        ffPrintTerminalHelpFormat,
        ffGenerateTerminalJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyTerminalOptions(FFTerminalOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
