#include "top.h"

#include <sys/types.h>
#include <sys/param.h> // DEV_BSIZE
#include <sys/sysctl.h>
#include <kvm.h>

const char* ffTopGetProcessSnapshot(FFlist* snapshots, FFTopTypes showTypes) {
    kvm_t* kd = kvm_open(nullptr, nullptr, nullptr, KVM_NO_FILES, nullptr);
    if (!kd) {
        return "kvm_open() failed";
    }

    int count = 0;
    // KERN_PROC_ALL returns all user-level processes, excluding kernel processes and threads
    const struct kinfo_proc* processes = kvm_getprocs(kd, KERN_PROC_ALL, 0, sizeof(struct kinfo_proc), &count);
    if (!processes) {
        kvm_close(kd);
        return "kvm_getprocs() failed";
    }

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
        int threadCount = 0;
        const struct kinfo_proc* threads = kvm_getprocs(kd,
            KERN_PROC_ALL | KERN_PROC_SHOW_THREADS, 0, sizeof(struct kinfo_proc), &threadCount);
        if (threads) {
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

    kvm_close(kd);
    return nullptr;
}
