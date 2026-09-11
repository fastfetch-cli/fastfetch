#include "processes.h"

#include "common/mallocHelper.h"

#include <sys/param.h>
#include <sys/sysctl.h>

const char* ffDetectProcesses(const FFProcessesOptions* options, FFProcessesResult* result) {
    int request[] = { CTL_KERN, KERN_PROC, (options->countKprocs ? KERN_PROC_KTHREAD : KERN_PROC_ALL) | KERN_PROC_SHOW_THREADS, 0, (int) sizeof(struct kinfo_proc), 0 };
    size_t length = 0;

    if (sysctl(request, ARRAY_SIZE(request), nullptr, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL}, nullptr) failed";
    }

    FF_AUTO_FREE struct kinfo_proc* procs = (struct kinfo_proc*) malloc(length);
    request[5] = (int) (length / sizeof(struct kinfo_proc)); // count must be non-zero for data fetch
    if (sysctl(request, ARRAY_SIZE(request), procs, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL}, procs) failed";
    }

    int count = (int) (length / sizeof(struct kinfo_proc));
    for (int i = 0; i < count; ++i) {
        const struct kinfo_proc* proc = &procs[i];

        ++result->threads;
        if (proc->p_tid == -1) {
            ++result->processes;
        }
    }

    return nullptr;
}
