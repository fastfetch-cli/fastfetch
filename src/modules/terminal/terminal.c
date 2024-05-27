#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/terminalshell/terminalshell.h"
#include "modules/terminal/terminal.h"
#include "util/stringUtils.h"

#include <string.h>

#define FF_TERMINAL_NUM_FORMAT_ARGS 8

void ffPrintTerminal(FFTerminalOptions* options)
{
    const FFTerminalResult* result = ffDetectTerminal();

    if(result->processName.length == 0)
    {
        ffPrintError(FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Couldn't detect terminal");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        if(result->version.length)
            printf("%s %s\n", result->prettyName.chars, result->version.chars);
        else
            ffStrbufPutTo(&result->prettyName, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_TERMINAL_NUM_FORMAT_ARGS, ((FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->processName, "process-name"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->exe, "exe"},
            {FF_FORMAT_ARG_TYPE_STRING, result->exeName, "exe-name"},
            {FF_FORMAT_ARG_TYPE_UINT, &result->pid, "pid"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->prettyName, "pretty-name"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->version, "version"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->exePath, "exe-path"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->tty, "tty"},
        }));
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

        ffPrintError(FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
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
    const FFTerminalResult* result = ffDetectTerminal();

    if(result->processName.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "Couldn't detect terminal");
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "processName", &result->processName);
    yyjson_mut_obj_add_strbuf(doc, obj, "exe", &result->exe);
    yyjson_mut_obj_add_strcpy(doc, obj, "exeName", result->exeName);
    yyjson_mut_obj_add_strbuf(doc, obj, "exePath", &result->exePath);
    yyjson_mut_obj_add_uint(doc, obj, "pid", result->pid);
    yyjson_mut_obj_add_uint(doc, obj, "ppid", result->ppid);
    yyjson_mut_obj_add_strbuf(doc, obj, "prettyName", &result->prettyName);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result->version);
    yyjson_mut_obj_add_strbuf(doc, obj, "tty", &result->tty);
}

void ffPrintTerminalHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_TERMINAL_MODULE_NAME, "{5} {6}", FF_TERMINAL_NUM_FORMAT_ARGS, ((const char* []) {
        "Terminal process name - process-name",
        "The first argument of the command line when running the terminal - exe",
        "Terminal base name of arg0 - exe-name",
        "Terminal pid - pid",
        "Terminal pretty name - pretty-name",
        "Terminal version - version",
        "Terminal full exe path - exe-path",
        "Terminal tty / pts used - tty",
    }));
}

void ffInitTerminalOptions(FFTerminalOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_TERMINAL_MODULE_NAME,
        "Print current terminal name and version",
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
