#include "fastfetch.h"
#include "common/printing.h"

#define FF_PROCESSES_MODULE_NAME "Processes"
#define FF_PROCESSES_NUM_FORMAT_ARGS 1

#ifdef __APPLE__
    #include <sys/sysctl.h>
#endif

void ffPrintProcesses(FFinstance* instance)
{
    #if FF_HAVE_SYSINFO_H
        uint16_t numProcesses = instance->state.sysinfo.procs;
    #else
        int request[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
        size_t length;

        if(sysctl(request, sizeof(request) / sizeof(*request), NULL, &length, NULL, 0) != 0)
        {
            ffPrintError(instance, FF_PROCESSES_MODULE_NAME, 0, &instance->config.processes, "sysctl() failed");
            return;
        }
        uint16_t numProcesses = (uint16_t)(length / sizeof(struct kinfo_proc));
    #endif

    if(instance->config.processes.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PROCESSES_MODULE_NAME, 0, &instance->config.processes.key);

        printf("%hu\n", numProcesses);
    }
    else
    {
        ffPrintFormat(instance, FF_PROCESSES_MODULE_NAME, 0, &instance->config.processes, FF_PROCESSES_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT16, &numProcesses}
        });
    }
}
