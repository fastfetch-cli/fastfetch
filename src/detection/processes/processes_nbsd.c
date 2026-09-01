#include "processes.h"
#include "common/mallocHelper.h"

#include <sys/sysctl.h>

const char* ffDetectProcesses(const FFProcessesOptions* options, FFProcessesResult* result) {
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

    for (uint32_t i = 0; i < procCount; ++i) {
        const struct kinfo_proc2* proc = &procs[i];
        if (!options->countKprocs && (proc->p_flag & P_SYSTEM)) {
            continue;
        }
        ++result->processes;

        int lwpRequest[] = { CTL_KERN, KERN_LWP, procs[i].p_pid, sizeof(struct kinfo_lwp), 0 };
        size_t lwpLength = 0;

        if (sysctl(lwpRequest, ARRAY_SIZE(lwpRequest), nullptr, &lwpLength, nullptr, 0) == 0) {
            result->threads += (uint32_t) (lwpLength / sizeof(struct kinfo_lwp));
        }
    }

    return nullptr;
}
