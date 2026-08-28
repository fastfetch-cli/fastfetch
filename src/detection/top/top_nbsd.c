#include "top.h"
#include "common/mallocHelper.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <errno.h>

const char* ffTopGetProcessSnapshot(FFlist* snapshots, FFTopTypes) {
    int request[] = { CTL_KERN, KERN_PROC2, KERN_PROC_ALL, -1, sizeof(struct kinfo_proc2), INT_MAX };
    size_t length;

    if (sysctl(request, ARRAY_SIZE(request), nullptr, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC2, KERN_PROC_ALL, nullptr}) failed";
    }

    // The process table may change between the two sysctl calls; retry with a larger buffer.
    length += length / 8 + sizeof(struct kinfo_proc2);

    FF_AUTO_FREE struct kinfo_proc2* processes = malloc(length);
    if (sysctl(request, ARRAY_SIZE(request), processes, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC2, KERN_PROC_ALL, processes}) failed";
    }
    uint32_t count = (uint32_t) (length / sizeof(struct kinfo_proc2));

    uint32_t pageSize = instance.state.platform.sysinfo.pageSize;

    for (uint32_t i = 0; i < count; ++i) {
        const struct kinfo_proc2* proc = &processes[i];

        if ((proc->p_flag & P_SYSTEM) || !proc->p_uvalid) { // Kernel process or zombie
            continue;
        }

        FFTopProcessSnapshot* item = FF_LIST_ADD(FFTopProcessSnapshot, *snapshots);

        ffStrbufInitS(&item->name, proc->p_comm);
        item->pid = (uint32_t) proc->p_pid;

        uint64_t userMs = (uint64_t) proc->p_uutime_sec * 1000 +
            (uint64_t) proc->p_uutime_usec / 1000;
        uint64_t sysMs = (uint64_t) proc->p_ustime_sec * 1000 +
            (uint64_t) proc->p_ustime_usec / 1000;
        item->cpuTime = userMs + sysMs;

        item->memBytes = (uint64_t) proc->p_vm_rssize * (uint64_t) pageSize;

        item->startTime = (uint64_t) proc->p_ustart_sec * 1000 +
            (uint64_t) proc->p_ustart_usec / 1000;

        item->bytesRead = (uint64_t) proc->p_uru_inblock * DEV_BSIZE;
        item->bytesWritten = (uint64_t) proc->p_uru_oublock * DEV_BSIZE;
    }

    return NULL;
}
