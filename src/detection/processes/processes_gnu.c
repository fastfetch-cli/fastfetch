#include "processes.h"

#include <hurd.h>
#include <ps.h>

const char* ffDetectProcesses(const FFProcessesOptions* options, FFProcessesResult* result) {
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

    proc_stat_list_set_flags(list, PSTAT_NUM_THREADS);

    for (uint32_t i = 0; i < list->num_procs; i++) {
        struct proc_stat* stat = list->proc_stats[i];
        if (!options->countKprocs && proc_stat_pid(stat) == 2) { // Kernel Task
            continue;
        }

        if (proc_stat_has(stat, PSTAT_NUM_THREADS)) {
            result->threads += proc_stat_num_threads(stat);
        }
    }

    result->processes = (uint32_t) list->num_procs;

done:
    if (list) {
        proc_stat_list_free(list);
    }
    ps_context_free(context);
    return error;
}
