#include "processes.h"

#include <sys/sysctl.h>
#ifdef __FreeBSD__
    #include <sys/types.h>
    #include <sys/user.h>
#endif

const char* ffDetectProcesses(uint32_t* result)
{
    int request[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    size_t length;

    if(sysctl(request, sizeof(request) / sizeof(*request), NULL, &length, NULL, 0) != 0)
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL}) failed";

    *result = (uint32_t)(length / sizeof(struct kinfo_proc));
    return NULL;
}
