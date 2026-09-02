#include "processes.h"
#include "common/mallocHelper.h"

#include <sys/sysctl.h>
#import <libproc.h>

const char* ffDetectProcesses(const FFProcessesOptions* options, FFProcessesResult* result) {
    int request[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL };
    size_t length;

    if (sysctl(request, ARRAY_SIZE(request), nullptr, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL, nullptr}) failed";
    }

    // The process table may change between the two sysctl calls; retry with a larger buffer.
    length += length / 8 + sizeof(struct kinfo_proc);
    FF_AUTO_FREE struct kinfo_proc* processes = malloc(length);

    if (sysctl(request, ARRAY_SIZE(request), processes, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL, processes}) failed";
    }

    uint32_t count = (uint32_t) (length / sizeof(struct kinfo_proc));

    for (uint32_t i = 0; i < count; ++i) {
        const struct kinfo_proc* proc = &processes[i];
        if (!options->countKprocs && (proc->kp_proc.p_flag & P_SYSTEM)) {
            continue;
        }
        result->processes++;

        pid_t pid = proc->kp_proc.p_pid;
        struct proc_taskinfo taskInfo;
        if (proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &taskInfo, sizeof(taskInfo)) != sizeof(taskInfo)) {
            continue;
        }
        result->threads += (uint32_t) taskInfo.pti_threadnum;
    }

    return nullptr;
}
