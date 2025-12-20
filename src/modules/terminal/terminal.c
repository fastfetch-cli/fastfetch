#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/terminalshell/terminalshell.h"
#include "modules/terminal/terminal.h"
#include "util/stringUtils.h"

bool ffPrintTerminal(FFTerminalOptions* options)
{
    const FFTerminalResult* result = ffDetectTerminal();

    if(result->processName.length == 0)
    {
        ffPrintError(FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Couldn't detect terminal");
        return false;
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
        FF_PRINT_FORMAT_CHECKED(FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(result->processName, "process-name"),
            FF_FORMAT_ARG(result->exe, "exe"),
            FF_FORMAT_ARG(result->exeName, "exe-name"),
            FF_FORMAT_ARG(result->pid, "pid"),
            FF_FORMAT_ARG(result->prettyName, "pretty-name"),
            FF_FORMAT_ARG(result->version, "version"),
            FF_FORMAT_ARG(result->exePath, "exe-path"),
            FF_FORMAT_ARG(result->tty, "tty"),
        }));
    }

    return true;
}

void ffParseTerminalJsonObject(FFTerminalOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_TERMINAL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateTerminalJsonConfig(FFTerminalOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateTerminalJsonResult(FF_MAYBE_UNUSED FFTerminalOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFTerminalResult* result = ffDetectTerminal();

    if(result->processName.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "Couldn't detect terminal");
        return false;
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

    return true;
}

void ffInitTerminalOptions(FFTerminalOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyTerminalOptions(FFTerminalOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffTerminalModuleInfo = {
    .name = FF_TERMINAL_MODULE_NAME,
    .description = "Print current terminal name and version",
    .initOptions = (void*) ffInitTerminalOptions,
    .destroyOptions = (void*) ffDestroyTerminalOptions,
    .parseJsonObject = (void*) ffParseTerminalJsonObject,
    .printModule = (void*) ffPrintTerminal,
    .generateJsonResult = (void*) ffGenerateTerminalJsonResult,
    .generateJsonConfig = (void*) ffGenerateTerminalJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Terminal process name", "process-name"},
        {"The first argument of the command line when running the terminal", "exe"},
        {"Terminal base name of arg0", "exe-name"},
        {"Terminal pid", "pid"},
        {"Terminal pretty name", "pretty-name"},
        {"Terminal version", "version"},
        {"Terminal full exe path", "exe-path"},
        {"Terminal tty / pts used", "tty"},
    }))
};
