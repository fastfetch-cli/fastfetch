#include "fastfetch.h"

#define FF_PROCESSES_MODULE_NAME "Processes"
#define FF_PROCESSES_NUM_FORMAT_ARGS 1

void ffPrintProcesses(FFinstance* instance)
{
    if(instance->config.processesFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PROCESSES_MODULE_NAME, 0, &instance->config.processesKey);

        printf("%hu\n", instance->state.sysinfo.procs);
    }
    else
    {
        ffPrintFormatString(instance, FF_PROCESSES_MODULE_NAME, 0, &instance->config.processesKey, &instance->config.processesFormat, NULL, FF_PROCESSES_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT16, &instance->state.sysinfo.procs}
        });
    }
}
