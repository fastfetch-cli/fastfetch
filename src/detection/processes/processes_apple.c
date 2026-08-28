#include "processes.h"
#include "common/mallocHelper.h"

#import <libproc.h>

const char* ffDetectProcesses(FFProcessesResult* result) {
    FF_AUTO_FREE pid_t* pids = malloc(sizeof(pid_t) * 0x1000);
    int count = proc_listallpids(pids, 0x1000);
    int trdCount = 0;
    for (int index = 0; index < count; ++index) {
        struct proc_taskinfo taskInfo;
        if (proc_pidinfo(pids[index], PROC_PIDTASKINFO, 0, &taskInfo, sizeof(taskInfo)) != sizeof(taskInfo)) {
            continue;
        }
        trdCount += taskInfo.pti_threadnum;
    }

    result->processes = (uint32_t) count;
    result->threads = (uint32_t) trdCount;
    return nullptr;
}
