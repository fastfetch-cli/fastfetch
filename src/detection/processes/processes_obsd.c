#include "processes.h"

#include <sys/param.h>
#include <sys/sysctl.h>

const char* ffDetectProcesses(FFProcessesResult* result) {
    int procRequest[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0, sizeof(struct kinfo_proc), 0 };
    size_t procLength = 0;

    if (sysctl(procRequest, ARRAY_SIZE(procRequest), nullptr, &procLength, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL}) failed";
    }

    result->processes = (uint32_t) (procLength / sizeof(struct kinfo_proc));

    int threadRequest[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL | KERN_PROC_SHOW_THREADS, 0, sizeof(struct kinfo_proc), 0 };
    size_t threadLength = 0;

    if (sysctl(threadRequest, ARRAY_SIZE(threadRequest), nullptr, &threadLength, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL | KERN_PROC_SHOW_THREADS}) failed";
    }

    result->threads = (uint32_t) (threadLength / sizeof(struct kinfo_proc));

    return nullptr;
}
