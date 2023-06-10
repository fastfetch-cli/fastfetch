#include "fastfetch.h"
#include "common/printing.h"
#include "detection/processes/processes.h"
#include "modules/processes/processes.h"

#define FF_PROCESSES_NUM_FORMAT_ARGS 1

void ffPrintProcesses(FFinstance* instance, FFProcessesOptions* options)
{
    uint32_t numProcesses = 0;
    const char* error = ffDetectProcesses(&numProcesses);

    if(error)
    {
        ffPrintError(instance, FF_PROCESSES_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PROCESSES_MODULE_NAME, 0, &options->moduleArgs.key);

        printf("%u\n", numProcesses);
    }
    else
    {
        ffPrintFormat(instance, FF_PROCESSES_MODULE_NAME, 0, &options->moduleArgs, FF_PROCESSES_NUM_FORMAT_ARGS, (FFformatarg[]){
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

#ifdef FF_HAVE_JSONC
void ffParseProcessesJsonObject(FFinstance* instance, json_object* module)
{
    FFProcessesOptions __attribute__((__cleanup__(ffDestroyProcessesOptions))) options;
    ffInitProcessesOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_PROCESSES_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintProcesses(instance, &options);
}
#endif
