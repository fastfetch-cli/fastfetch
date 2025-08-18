#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/terminalshell/terminalshell.h"
#include "modules/shell/shell.h"
#include "util/stringUtils.h"

bool ffPrintShell(FFShellOptions* options)
{
    const FFShellResult* result = ffDetectShell();

    if(result->processName.length == 0)
    {
        ffPrintError(FF_SHELL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Couldn't detect shell");
        return false;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_SHELL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&result->prettyName, stdout);

        if(result->version.length > 0)
        {
            putchar(' ');
            ffStrbufWriteTo(&result->version, stdout);
        }

        putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_SHELL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(result->processName, "process-name"),
            FF_FORMAT_ARG(result->exe, "exe"),
            FF_FORMAT_ARG(result->exeName, "exe-name"),
            FF_FORMAT_ARG(result->version, "version"),
            FF_FORMAT_ARG(result->pid, "pid"),
            FF_FORMAT_ARG(result->prettyName, "pretty-name"),
            FF_FORMAT_ARG(result->exePath, "exe-path"),
            FF_FORMAT_ARG(result->tty, "tty"),
        }));
    }

    return true;
}

void ffParseShellJsonObject(FFShellOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_SHELL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateShellJsonConfig(FFShellOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateShellJsonResult(FF_MAYBE_UNUSED FFShellOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFShellResult* result = ffDetectShell();

    if(result->processName.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "Couldn't detect shell");
        return false;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "exe", &result->exe);
    yyjson_mut_obj_add_strcpy(doc, obj, "exeName", result->exeName);
    yyjson_mut_obj_add_strbuf(doc, obj, "exePath", &result->exePath);
    yyjson_mut_obj_add_uint(doc, obj, "pid", result->pid);
    yyjson_mut_obj_add_uint(doc, obj, "ppid", result->ppid);
    yyjson_mut_obj_add_strbuf(doc, obj, "processName", &result->processName);
    yyjson_mut_obj_add_strbuf(doc, obj, "prettyName", &result->prettyName);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result->version);
    if (result->tty >= 0)
        yyjson_mut_obj_add_int(doc, obj, "tty", result->tty);
    else
        yyjson_mut_obj_add_null(doc, obj, "tty");

    return true;
}

void ffInitShellOptions(FFShellOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyShellOptions(FFShellOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffShellModuleInfo = {
    .name = FF_SHELL_MODULE_NAME,
    .description = "Print current shell name and version",
    .initOptions = (void*) ffInitShellOptions,
    .destroyOptions = (void*) ffDestroyShellOptions,
    .parseJsonObject = (void*) ffParseShellJsonObject,
    .printModule = (void*) ffPrintShell,
    .generateJsonResult = (void*) ffGenerateShellJsonResult,
    .generateJsonConfig = (void*) ffGenerateShellJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Shell process name", "process-name"},
        {"The first argument of the command line when running the shell", "exe"},
        {"Shell base name of arg0", "exe-name"},
        {"Shell version", "version"},
        {"Shell pid", "pid"},
        {"Shell pretty name", "pretty-name"},
        {"Shell full exe path", "exe-path"},
        {"Shell tty used", "tty"},
    }))
};
