#include "top.h"
#include "common/mallocHelper.h"

#include <errno.h>
#include <sys/sysctl.h>
#include <libproc.h>

const char* ffTopGetProcessSnapshot(FFlist* snapshots, FFTopTypes showTypes) {
    int npids = proc_listallpids(nullptr, 0);
    if (npids <= 0) {
        return "proc_listallpids(nullptr, 0) failed";
    }
    // `proc_listallpids` returns the number of pids, but it wants the size of the buffer in bytes:
    // passing the count back would only let the kernel fill a quarter of it.
    const int pidCapacity = npids + npids / 8 + 1;
    FF_AUTO_FREE pid_t* pids = malloc((size_t) pidCapacity * sizeof(pid_t));
    if (pids == nullptr) {
        return "malloc() failed";
    }
    npids = proc_listallpids(pids, pidCapacity * (int) sizeof(pid_t));
    if (npids <= 0) {
        return "proc_listallpids(pids, bufferSize) failed";
    }

    uint32_t count = (uint32_t) npids;

    for (uint32_t i = 0; i < count; ++i) {
        pid_t pid = pids[i];

        struct proc_taskallinfo proc;
        if (proc_pidinfo(pid, PROC_PIDTASKALLINFO, 0, &proc, sizeof(proc)) != sizeof(proc)) {
            continue;
        }

        if (proc.pbsd.pbi_flags & PROC_FLAG_SYSTEM) {
            continue;
        }

        FFTopProcessSnapshot* item = FF_LIST_ADD(FFTopProcessSnapshot, *snapshots);
        ffStrbufInitS(&item->name, proc.pbsd.pbi_name);
        if (item->name.length == 0) {
            ffStrbufInitS(&item->name, proc.pbsd.pbi_comm);
        }
        item->pid = (uint32_t) pid;
        item->cpuTime = (proc.ptinfo.pti_total_user + proc.ptinfo.pti_total_system) / 1000000u; // ns -> ms
        item->memBytes = proc.ptinfo.pti_resident_size;
        item->startTime = proc.pbsd.pbi_start_tvsec * 1000u + proc.pbsd.pbi_start_tvusec / 1000u; // convert to ms
        item->threads = (uint32_t) proc.ptinfo.pti_threadnum;

        // FF_LIST_ADD does not zero the element, and top.c subtracts both counters for every
        // process it samples, whether or not they were collected.
        item->bytesRead = 0;
        item->bytesWritten = 0;
        if (showTypes & FF_TOP_TYPE_DISK) {
            struct rusage_info_v2 rusage;
            if (proc_pid_rusage(pid, RUSAGE_INFO_V2, (rusage_info_t*) &rusage) == 0) {
                item->bytesRead = rusage.ri_diskio_bytesread;
                item->bytesWritten = rusage.ri_diskio_byteswritten;
            }
        }
    }

    return nullptr;
}
