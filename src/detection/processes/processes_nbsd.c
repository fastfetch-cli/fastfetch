#include "processes.h"
#include "common/mallocHelper.h"

#include <sys/sysctl.h>

const char* ffDetectProcesses(FFProcessesResult* result) {
    int request[] = { CTL_KERN, KERN_PROC2, KERN_PROC_ALL, -1, sizeof(struct kinfo_proc2), INT_MAX };
    size_t length = 0;

    if (sysctl(request, ARRAY_SIZE(request), nullptr, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC2, KERN_PROC_ALL, nullptr}) failed";
    }

    FF_AUTO_FREE struct kinfo_proc2* procs = malloc(length);
    if (sysctl(request, ARRAY_SIZE(request), procs, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC2, KERN_PROC_ALL, procs}) failed";
    }

    uint32_t procCount = (uint32_t) (length / sizeof(struct kinfo_proc2));
    result->processes = procCount;

    uint32_t threads = 0;
    for (uint32_t i = 0; i < procCount; ++i) {
        int lwpRequest[] = { CTL_KERN, KERN_LWP, procs[i].p_pid, sizeof(struct kinfo_lwp), 0 };
        size_t lwpLength = 0;

        if (sysctl(lwpRequest, ARRAY_SIZE(lwpRequest), nullptr, &lwpLength, nullptr, 0) == 0) {
            threads += (uint32_t) (lwpLength / sizeof(struct kinfo_lwp));
        }
    }

    result->threads = threads;
    return nullptr;
}
