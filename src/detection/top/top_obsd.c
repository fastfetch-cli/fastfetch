#include "top.h"

#include "common/mallocHelper.h"

#include <sys/types.h>
#include <sys/param.h> // DEV_BSIZE
#include <sys/sysctl.h>

const char* ffTopGetProcessSnapshot(FFlist* snapshots, FFTopTypes showTypes) {
    int request[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0, (int) sizeof(struct kinfo_proc), 0 };
    size_t length = 0;

    if (sysctl(request, ARRAY_SIZE(request), nullptr, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL}, nullptr) failed";
    }

    FF_AUTO_FREE struct kinfo_proc* processes = (struct kinfo_proc*) malloc(length);
    request[5] = (int) (length / sizeof(struct kinfo_proc)); // count must be non-zero for data fetch
    if (sysctl(request, ARRAY_SIZE(request), processes, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL}, processes) failed";
    }

    int count = (int) (length / sizeof(struct kinfo_proc));
    const uint32_t pageSize = instance.state.platform.sysinfo.pageSize;

    for (int i = 0; i < count; ++i) {
        const struct kinfo_proc* proc = &processes[i];

        if (!proc->p_uvalid) { // Zombie process; p_u* members are invalid
            continue;
        }

        FFTopProcessSnapshot* item = FF_LIST_ADD(FFTopProcessSnapshot, *snapshots);
        ffStrbufInitS(&item->name, proc->p_comm);
        item->pid = (uint32_t) proc->p_pid;
        item->cpuTime = ((uint64_t) proc->p_uutime_sec * 1000 + (uint64_t) proc->p_uutime_usec / 1000) +
            ((uint64_t) proc->p_ustime_sec * 1000 + (uint64_t) proc->p_ustime_usec / 1000);
        item->memBytes = (uint64_t) proc->p_vm_rssize * pageSize;
        item->startTime = (uint64_t) proc->p_ustart_sec * 1000 + (uint64_t) proc->p_ustart_usec / 1000;
        item->bytesRead = (uint64_t) proc->p_uru_inblock * DEV_BSIZE;
        item->bytesWritten = (uint64_t) proc->p_uru_oublock * DEV_BSIZE;
        item->threads = 0;
    }

    if (showTypes & FF_TOP_TYPE_THREADS) {
        int threadRequest[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL | KERN_PROC_SHOW_THREADS, 0, (int) sizeof(struct kinfo_proc), 0 };
        size_t threadLength = 0;
        if (sysctl(threadRequest, ARRAY_SIZE(threadRequest), nullptr, &threadLength, nullptr, 0) == 0 && threadLength > 0) {
            FF_AUTO_FREE struct kinfo_proc* threads = (struct kinfo_proc*) malloc(threadLength);
            threadRequest[5] = (int) (threadLength / sizeof(struct kinfo_proc)); // count must be non-zero for data fetch
            if (sysctl(threadRequest, ARRAY_SIZE(threadRequest), threads, &threadLength, nullptr, 0) == 0) {
                int threadCount = (int) (threadLength / sizeof(struct kinfo_proc));
                for (uint32_t i = 0; i < snapshots->length; ++i) {
                    FFTopProcessSnapshot* item = FF_LIST_GET(FFTopProcessSnapshot, *snapshots, i);
                    for (int j = 0; j < threadCount; ++j) {
                        if (threads[j].p_pid == (pid_t) item->pid && threads[j].p_tid != -1) {
                            ++item->threads;
                        }
                    }
                }
            }
        }
    }

    return nullptr;
}
