#include "processes.h"

#include <sys/sysctl.h>
#ifdef __FreeBSD__
    #include <sys/types.h>
    #include <sys/user.h>
#endif

#ifndef KERN_PROC_PROC
    #define KERN_PROC_PROC KERN_PROC_ALL // Apple
#endif

const char* ffDetectProcesses(uint32_t* result)
{
    int request[] = {CTL_KERN, KERN_PROC, KERN_PROC_PROC};
    size_t length;

    if(sysctl(request, ARRAY_SIZE(request), NULL, &length, NULL, 0) != 0)
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_PROC}) failed";

    *result = (uint32_t)(length / sizeof(struct kinfo_proc));
    return NULL;
}
