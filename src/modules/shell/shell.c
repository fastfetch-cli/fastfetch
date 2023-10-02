#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/terminalshell/terminalshell.h"
#include "modules/shell/shell.h"
#include "util/stringUtils.h"

#define FF_SHELL_NUM_FORMAT_ARGS 6

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
            {FF_FORMAT_ARG_TYPE_UINT, &result->shellPid},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->shellPrettyName},
        });
    }
}

void ffInitShellOptions(FFShellOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_SHELL_MODULE_NAME, ffParseShellCommandOptions, ffParseShellJsonObject, ffPrintShell, ffGenerateShellJson);
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

void ffParseShellJsonObject(FFShellOptions* options, yyjson_val* module)
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

        ffPrintError(FF_SHELL_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateShellJson(FF_MAYBE_UNUSED FFShellOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFTerminalShellResult* result = ffDetectTerminalShell();

    if(result->shellProcessName.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "Couldn't detect shell");
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "exe", &result->shellExe);
    yyjson_mut_obj_add_strcpy(doc, obj, "exeName", result->shellExeName);
    yyjson_mut_obj_add_uint(doc, obj, "pid", result->shellPid);
    yyjson_mut_obj_add_strbuf(doc, obj, "processName", &result->shellProcessName);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result->shellVersion);
}
