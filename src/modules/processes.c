#include "fastfetch.h"
#include "common/printing.h"
#include "detection/processes/processes.h"

#define FF_PROCESSES_MODULE_NAME "Processes"
#define FF_PROCESSES_NUM_FORMAT_ARGS 1

void ffPrintProcesses(FFinstance* instance)
{
    uint32_t numProcesses = 0;
    const char* error = ffDetectProcesses(&numProcesses);

    if(error)
    {
        ffPrintError(instance, FF_PROCESSES_MODULE_NAME, 0, &instance->config.processes, "%s", error);
        return;
    }

    if(instance->config.processes.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PROCESSES_MODULE_NAME, 0, &instance->config.processes.key);

        printf("%u\n", numProcesses);
    }
    else
    {
        ffPrintFormat(instance, FF_PROCESSES_MODULE_NAME, 0, &instance->config.processes, FF_PROCESSES_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT, &numProcesses}
        });
    }
}
