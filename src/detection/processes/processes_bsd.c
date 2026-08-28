#include "processes.h"

#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>

const char* ffDetectProcesses(FFProcessesResult* result) {
    int procRequest[] = { CTL_KERN, KERN_PROC, KERN_PROC_PROC };
    size_t procLength;
    if (sysctl(procRequest, ARRAY_SIZE(procRequest), nullptr, &procLength, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_PROC}) failed";
    }

    int threadRequest[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL };
    size_t threadLength;
    if (sysctl(threadRequest, ARRAY_SIZE(threadRequest), nullptr, &threadLength, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL}) failed";
    }

    result->processes = (uint32_t) (procLength / sizeof(struct kinfo_proc));
    result->threads = (uint32_t) (threadLength / sizeof(struct kinfo_proc));
    return nullptr;
}
