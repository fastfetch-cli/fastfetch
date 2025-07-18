#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/processes/processes.h"
#include "modules/processes/processes.h"
#include "util/stringUtils.h"

void ffPrintProcesses(FFProcessesOptions* options)
{
    uint32_t numProcesses = 0;
    const char* error = ffDetectProcesses(&numProcesses);

    if(error)
    {
        ffPrintError(FF_PROCESSES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_PROCESSES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        printf("%u\n", numProcesses);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_PROCESSES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(numProcesses, "result")
        }));
    }
}

bool ffParseProcessesCommandOptions(FFProcessesOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_PROCESSES_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseProcessesJsonObject(FFProcessesOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type") || ffStrEqualsIgnCase(key, "condition"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_PROCESSES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateProcessesJsonConfig(FFProcessesOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyProcessesOptions))) FFProcessesOptions defaultOptions;
    ffInitProcessesOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateProcessesJsonResult(FF_MAYBE_UNUSED FFProcessesOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    uint32_t result;
    const char* error = ffDetectProcesses(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_obj_add_uint(doc, module, "result", result);
}

static FFModuleBaseInfo ffModuleInfo = {
    .name = FF_PROCESSES_MODULE_NAME,
    .description = "Print number of running processes",
    .parseCommandOptions = (void*) ffParseProcessesCommandOptions,
    .parseJsonObject = (void*) ffParseProcessesJsonObject,
    .printModule = (void*) ffPrintProcesses,
    .generateJsonResult = (void*) ffGenerateProcessesJsonResult,
    .generateJsonConfig = (void*) ffGenerateProcessesJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Process count", "result"}
    }))
};

void ffInitProcessesOptions(FFProcessesOptions* options)
{
    options->moduleInfo = ffModuleInfo;
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyProcessesOptions(FFProcessesOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
