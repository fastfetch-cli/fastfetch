#include "top.h"

#include <hurd.h>
#include <string.h>
#include <ps.h>

const char* ffTopGetProcessSnapshot(FFlist* snapshots, FFTopTypes showTypes) {
    struct ps_context* context = nullptr;
    if (ps_context_create(getproc(), &context) != 0) {
        return "ps_context_create(getproc()) failed";
    }

    const char* error = nullptr;
    struct proc_stat_list* list = nullptr;
    if (proc_stat_list_create(context, &list) != 0) {
        error = "proc_stat_list_create() failed";
        goto done;
    }

    if (proc_stat_list_add_all(list, nullptr, nullptr) != 0) {
        error = "proc_stat_list_add_all() failed";
        goto done;
    }

    // Flags that cannot be fetched for some process are simply left unset;
    // each field must be checked before use.
    if (proc_stat_list_set_flags(list, PSTAT_ARGS | PSTAT_STATE | PSTAT_TASK_BASIC | ((showTypes & FF_TOP_TYPE_THREADS) ? PSTAT_NUM_THREADS : 0)) != 0) {
        error = "proc_stat_list_set_flags() failed";
        goto done;
    }

    for (unsigned i = 0; i < list->num_procs; ++i) {
        struct proc_stat* stat = list->proc_stats[i];

        if (!proc_stat_has(stat, PSTAT_PID | PSTAT_STATE)) {
            continue;
        }
        if (proc_stat_state(stat) & PSTAT_STATE_P_ZOMBIE) {
            continue;
        }

        uint32_t pid = (uint32_t) proc_stat_pid(stat);
        if (pid == 2) { // Kernel Task
            continue;
        }

        FFTopProcessSnapshot* item = FF_LIST_ADD(FFTopProcessSnapshot, *snapshots);

        // There is no kernel-side comm on the Hurd; use the first argument.
        ffStrbufInit(&item->name);
        if (proc_stat_has(stat, PSTAT_ARGS) && proc_stat_args(stat)) {
            size_t len = strnlen(proc_stat_args(stat), proc_stat_args_len(stat));
            ffStrbufAppendNS(&item->name, (uint32_t) len, proc_stat_args(stat));
        }
        if (item->name.length == 0) {
            ffStrbufAppendS(&item->name, "(unknown)");
        }

        item->pid = pid;
        item->cpuTime = 0;
        item->memBytes = 0;
        item->startTime = 0;
        item->threads = 0;

        if (showTypes & FF_TOP_TYPE_THREADS) {
            if (proc_stat_has(stat, PSTAT_NUM_THREADS)) {
                item->threads = (uint32_t) proc_stat_num_threads(stat);
            }
        }

        if (proc_stat_has(stat, PSTAT_TASK_BASIC)) {
            const task_basic_info_t info = proc_stat_task_basic_info(stat);
            item->cpuTime = (uint64_t) info->user_time64.seconds * 1000 +
                (uint64_t) info->user_time64.nanoseconds / 1000000 +
                (uint64_t) info->system_time64.seconds * 1000 +
                (uint64_t) info->system_time64.nanoseconds / 1000000;
            item->memBytes = (uint64_t) info->resident_size;
            item->startTime = (uint64_t) info->creation_time64.seconds * 1000 +
                (uint64_t) info->creation_time64.nanoseconds / 1000000;
        }

        // Storage io counters aren't exposed by kernel; leave them zeroed.
        item->bytesRead = 0;
        item->bytesWritten = 0;
    }

done:
    if (list) {
        proc_stat_list_free(list);
    }
    ps_context_free(context);
    return error;
}
