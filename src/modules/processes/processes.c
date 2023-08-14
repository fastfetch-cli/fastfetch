#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/processes/processes.h"
#include "modules/processes/processes.h"
#include "util/stringUtils.h"

#define FF_PROCESSES_NUM_FORMAT_ARGS 1

void ffPrintProcesses(FFProcessesOptions* options)
{
    uint32_t numProcesses = 0;
    const char* error = ffDetectProcesses(&numProcesses);

    if(error)
    {
        ffPrintError(FF_PROCESSES_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_PROCESSES_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);

        printf("%u\n", numProcesses);
    }
    else
    {
        ffPrintFormat(FF_PROCESSES_MODULE_NAME, 0, &options->moduleArgs, FF_PROCESSES_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT, &numProcesses}
        });
    }
}

void ffInitProcessesOptions(FFProcessesOptions* options)
{
    options->moduleName = FF_PROCESSES_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseProcessesCommandOptions(FFProcessesOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_PROCESSES_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyProcessesOptions(FFProcessesOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseProcessesJsonObject(yyjson_val* module)
{
    FFProcessesOptions __attribute__((__cleanup__(ffDestroyProcessesOptions))) options;
    ffInitProcessesOptions(&options);

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

            ffPrintError(FF_PROCESSES_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintProcesses(&options);
}
