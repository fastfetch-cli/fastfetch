#include "top.h"
#include "common/mallocHelper.h"

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/user.h>

const char* ffTopGetProcessSnapshot(FFlist* snapshots, FFTopTypes) {
    int request[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL };
    size_t length;

    if (sysctl(request, ARRAY_SIZE(request), nullptr, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL}) failed";
    }

    // The process table may change between the two sysctl calls; retry with a larger buffer.
    FF_AUTO_FREE struct kinfo_proc* processes = nullptr;
    for (int attempts = 0;; ++attempts) {
        length += length / 8 + sizeof(struct kinfo_proc);
        struct kinfo_proc* newProcesses = (struct kinfo_proc*) realloc(processes, length);
        if (!newProcesses) {
            return "realloc(struct kinfo_proc[]) failed";
        }
        processes = newProcesses;
        if (sysctl(request, ARRAY_SIZE(request), processes, &length, nullptr, 0) == 0) {
            break;
        }
        if ((errno != ENOMEM && errno != EINVAL) || attempts >= 4) {
            return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL}) failed";
        }
    }
    uint32_t count = (uint32_t) (length / sizeof(struct kinfo_proc));

    uint32_t pageSize = instance.state.platform.sysinfo.pageSize;

    for (uint32_t i = 0; i < count; ++i) {
        const struct kinfo_proc* proc = &processes[i];

        // Kernel threads are reported with P_SYSTEM flags and pid -1.
        if ((proc->kp_flags & P_SYSTEM) || proc->kp_stat == SZOMB) {
            continue;
        }

        FFTopProcessSnapshot* item = FF_LIST_ADD(FFTopProcessSnapshot, *snapshots);

        ffStrbufInitS(&item->name, proc->kp_comm);
        item->pid = (uint32_t) proc->kp_pid;

        // kp_ru only accounts for exited lwps; the cpu times of live lwps are
        // kept in microseconds in kp_lwp.kl_uticks/kl_sticks (see calcru(9)).
        uint64_t exitedMs = (uint64_t) (proc->kp_ru.ru_utime.tv_sec + proc->kp_ru.ru_stime.tv_sec) * 1000 +
            (uint64_t) (proc->kp_ru.ru_utime.tv_usec + proc->kp_ru.ru_stime.tv_usec) / 1000;
        uint64_t liveMs = (proc->kp_lwp.kl_uticks + proc->kp_lwp.kl_sticks) / 1000;
        item->cpuTime = exitedMs + liveMs;

        item->memBytes = (uint64_t) proc->kp_vm_rssize * pageSize;

        item->startTime = (uint64_t) proc->kp_start.tv_sec * 1000 +
            (uint64_t) proc->kp_start.tv_usec / 1000;

        item->bytesRead = (uint64_t) (proc->kp_ru.ru_inblock + proc->kp_lwp.kl_ru.ru_inblock) * DEV_BSIZE;
        item->bytesWritten = (uint64_t) (proc->kp_ru.ru_oublock + proc->kp_lwp.kl_ru.ru_oublock) * DEV_BSIZE;
    }

    return nullptr;
}
