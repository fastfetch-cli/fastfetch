#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/processes/processes.h"
#include "modules/processes/processes.h"
#include "util/stringUtils.h"

bool ffPrintProcesses(FFProcessesOptions* options)
{
    uint32_t numProcesses = 0;
    const char* error = ffDetectProcesses(&numProcesses);

    if(error)
    {
        ffPrintError(FF_PROCESSES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
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

    return true;
}

void ffParseProcessesJsonObject(FFProcessesOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_PROCESSES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateProcessesJsonConfig(FFProcessesOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateProcessesJsonResult(FF_MAYBE_UNUSED FFProcessesOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    uint32_t result;
    const char* error = ffDetectProcesses(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_obj_add_uint(doc, module, "result", result);

    return true;
}

void ffInitProcessesOptions(FFProcessesOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyProcessesOptions(FFProcessesOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffProcessesModuleInfo = {
    .name = FF_PROCESSES_MODULE_NAME,
    .description = "Print number of running processes",
    .initOptions = (void*) ffInitProcessesOptions,
    .destroyOptions = (void*) ffDestroyProcessesOptions,
    .parseJsonObject = (void*) ffParseProcessesJsonObject,
    .printModule = (void*) ffPrintProcesses,
    .generateJsonResult = (void*) ffGenerateProcessesJsonResult,
    .generateJsonConfig = (void*) ffGenerateProcessesJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Process count", "result"}
    }))
};
