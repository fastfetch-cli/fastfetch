#include "processes.h"
#include "common/mallocHelper.h"

#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>

const char* ffDetectProcesses(const FFProcessesOptions* options, FFProcessesResult* result) {
    int request[] = { CTL_KERN, KERN_PROC, KERN_PROC_PROC };
    size_t length;
    if (sysctl(request, ARRAY_SIZE(request), nullptr, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_PROC}) failed";
    }

    // The process table may change between the two sysctl calls; retry with a larger buffer.
    length += length / 8 + sizeof(struct kinfo_proc);
    FF_AUTO_FREE struct kinfo_proc* procs = malloc(length);
    if (sysctl(request, ARRAY_SIZE(request), procs, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_PROC}) failed";
    }
    
    uint32_t count = (uint32_t) (length / sizeof(struct kinfo_proc));
    for (uint32_t i = 0; i < count; ++i) {
        const struct kinfo_proc* proc = &procs[i];

        if (!options->countKprocs && (proc->ki_flag & P_KPROC)) {
            continue;
        }

        result->processes++;
        result->threads += (uint32_t) proc->ki_numthreads;
    }

    return nullptr;
}
