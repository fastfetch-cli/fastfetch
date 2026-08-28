#include "top.h"
#include "common/mallocHelper.h"

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/user.h>

const char* ffTopGetProcessSnapshot(FFlist* snapshots, FFTopTypes) {
    int request[] = { CTL_KERN, KERN_PROC, KERN_PROC_PROC };
    size_t length;

    if (sysctl(request, ARRAY_SIZE(request), nullptr, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_PROC}) failed";
    }

    // The process table may change between the two sysctl calls; retry with a larger buffer.
    length += length / 8 + sizeof(struct kinfo_proc);
    FF_AUTO_FREE struct kinfo_proc* processes = malloc(length);
    if (sysctl(request, ARRAY_SIZE(request), processes, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_PROC}) failed";
    }
    uint32_t count = (uint32_t) (length / sizeof(struct kinfo_proc));

    uint32_t pageSize = instance.state.platform.sysinfo.pageSize;

    for (uint32_t i = 0; i < count; ++i) {
        const struct kinfo_proc* proc = &processes[i];

        if (proc->ki_flag & P_KPROC) {
            continue;
        }

        uint32_t pid = (uint32_t) proc->ki_pid;

        FFTopProcessSnapshot* item = FF_LIST_ADD(FFTopProcessSnapshot, *snapshots);

        ffStrbufInitS(&item->name, proc->ki_comm);
        item->pid = pid;

        uint64_t userMs = (uint64_t) proc->ki_rusage.ru_utime.tv_sec * 1000 +
            (uint64_t) proc->ki_rusage.ru_utime.tv_usec / 1000;
        uint64_t sysMs = (uint64_t) proc->ki_rusage.ru_stime.tv_sec * 1000 +
            (uint64_t) proc->ki_rusage.ru_stime.tv_usec / 1000;
        item->cpuTime = userMs + sysMs;

        item->memBytes = (uint64_t) proc->ki_rssize * (uint64_t) pageSize;

        item->startTime = (uint64_t) proc->ki_start.tv_sec * 1000 +
            (uint64_t) proc->ki_start.tv_usec / 1000;

        item->bytesRead = (uint64_t) proc->ki_rusage.ru_inblock * DEV_BSIZE;
        item->bytesWritten = (uint64_t) proc->ki_rusage.ru_oublock * DEV_BSIZE;
    }

    return NULL;
}
