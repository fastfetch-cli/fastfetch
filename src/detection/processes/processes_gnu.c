#include "processes.h"

#include <hurd.h>
#include <ps.h>

const char* ffDetectProcesses(FFProcessesResult* result) {
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

    struct proc_stat **stats = nullptr;
    unsigned int numProcs = 0;
    if (proc_stat_list_add_all(list, &stats, &numProcs) != 0) {
        error = "proc_stat_list_add_all() failed";
        goto done;
    }

    uint32_t nthread = 0;
    for (unsigned int i = 0; i < numProcs; i++) {
        proc_stat_get_basic_info(stats[i]);
        nthread += proc_stat_thread_count(stats[i]);
    }

    result->processes = (uint32_t) numProcs;
    result->threads = nthread;

done:
    if (stats) {
        proc_stat_list_free_stats(stats, numProcs);
    }
    if (list) {
        proc_stat_list_free(list);
    }
    ps_context_free(context);
    return error;
}
