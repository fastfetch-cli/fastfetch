#include "top.h"
#include "common/mallocHelper.h"

#include <errno.h>
#include <sys/sysctl.h>
#include <libproc.h>

const char* ffTopGetProcessSnapshot(FFlist* snapshots, FFTopTypes showTypes) {
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
        if (proc->kp_proc.p_flag & P_SYSTEM) {
            continue;
        }
        pid_t pid = proc->kp_proc.p_pid;

        struct rusage_info_v2 rusage;
        if (proc_pid_rusage(pid, RUSAGE_INFO_V2, (rusage_info_t*) &rusage) != 0) {
            continue; // The process may have exited
        }

        FFTopProcessSnapshot* item = FF_LIST_ADD(FFTopProcessSnapshot, *snapshots);
        ffStrbufInitS(&item->name, proc->kp_proc.p_comm);
        item->pid = (uint32_t) pid;
        // Note: Do NOT use proc->kp_proc.p_pctcpu for CPU usage. p_pctcpu is a
        // decaying average (fixpt_t with FSCALE=2048) updated roughly once per
        // second by the kernel. It is heavily smoothed, lags short bursts, has
        // low resolution, and its multicore scaling (>100%) is inconsistent
        // across XNU versions. It also breaks the unified model where
        // FFTopProcessSnapshot.cpuTime is cumulative time and top.c computes
        // (new - old) / elapsed * 100 for all platforms. proc_pid_rusage with
        // RUSAGE_INFO_V2 provides precise cumulative ri_user_time +
        // ri_system_time in nanoseconds and is the modern recommended API on
        // Darwin, consistent with Linux/BSD differential sampling and accurate
        // for the short waitTime interval (e.g. 100ms).
        item->cpuTime = (rusage.ri_user_time + rusage.ri_system_time) / 1000000u; // ns -> ms
        item->memBytes = rusage.ri_resident_size;
        item->bytesRead = rusage.ri_diskio_bytesread;
        item->bytesWritten = rusage.ri_diskio_byteswritten;
        item->startTime = rusage.ri_proc_start_abstime;
        item->threads = 0;
        if (showTypes & FF_TOP_TYPE_THREADS) {
            struct proc_taskinfo taskInfo;
            if (proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &taskInfo, sizeof(taskInfo)) == sizeof(taskInfo)) {
                item->threads = (uint32_t) taskInfo.pti_threadnum;
            }
        }
    }

    return nullptr;
}
