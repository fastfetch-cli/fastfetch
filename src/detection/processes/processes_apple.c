#include "processes.h"

#include <sys/sysctl.h>

uint32_t ffDetectProcesses(FFinstance* instance, FFstrbuf* error)
{
    FF_UNUSED(instance);

    int request[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    size_t length;

    if(sysctl(request, sizeof(request) / sizeof(*request), NULL, &length, NULL, 0) != 0)
    {
        ffStrbufAppendS(error, "sysctl() failed");
        return 0;
    }
    return (uint32_t)(length / sizeof(struct kinfo_proc));
}
