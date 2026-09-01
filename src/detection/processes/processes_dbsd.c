#include "processes.h"
#include "common/mallocHelper.h"

#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/proc.h>

const char* ffDetectProcesses(const FFProcessesOptions* options, FFProcessesResult* result) {
    int request[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL | (options->countKprocs ? KERN_PROC_FLAG_LWKT : 0) };
    size_t length;
    if (sysctl(request, ARRAY_SIZE(request), nullptr, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL}) failed";
    }

    // The process table may change between the two sysctl calls; retry with a larger buffer.
    length += length / 8 + sizeof(struct kinfo_proc);
    FF_AUTO_FREE struct kinfo_proc* procs = malloc(length);
    if (sysctl(request, ARRAY_SIZE(request), procs, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL}) failed";
    }

    uint32_t count = (uint32_t) (length / sizeof(struct kinfo_proc));
    for (uint32_t i = 0; i < count; ++i) {
        const struct kinfo_proc* proc = &procs[i];

        if (proc->kp_pid == -1) {
            // Kernel thread, only returned when countKprocs is set via KERN_PROC_FLAG_LWKT
            result->threads++;
            continue;
        }

        result->processes++;
        result->threads += (uint32_t) proc->kp_nthreads;
    }

    return nullptr;
}
