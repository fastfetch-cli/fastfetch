#include "fastfetch.h"
#include "common/printing.h"

#define FF_PROCESSES_MODULE_NAME "Processes"
#define FF_PROCESSES_NUM_FORMAT_ARGS 1

void ffPrintProcesses(FFinstance* instance)
{
    if(instance->config.processes.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PROCESSES_MODULE_NAME, 0, &instance->config.processes.key);

        printf("%hu\n", instance->state.sysinfo.procs);
    }
    else
    {
        ffPrintFormat(instance, FF_PROCESSES_MODULE_NAME, 0, &instance->config.processes, FF_PROCESSES_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT16, &instance->state.sysinfo.procs}
        });
    }
}
