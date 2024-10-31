#include "processes.h"

#include <sys/sysctl.h>

const char* ffDetectProcesses(uint32_t* result)
{
    int request[] = {CTL_KERN, KERN_PROC2, KERN_PROC_ALL, -1, sizeof(struct kinfo_proc2), 0};
    size_t length = 0;

    if(sysctl(request, ARRAY_SIZE(request), NULL, &length, NULL, 0) != 0)
        return "sysctl({CTL_KERN, KERN_PROC2, KERN_PROC_ALL}) failed";

    *result = (uint32_t)(length / sizeof(struct kinfo_proc2));
    return NULL;
}
