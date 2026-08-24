#include "processes.h"

#include <hurd.h>
#include <ps.h>

const char* ffDetectProcesses(uint32_t* result) {
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

    // No flags are needed; we only count the processes.
    if (proc_stat_list_add_all(list, nullptr, nullptr) != 0) {
        error = "proc_stat_list_add_all() failed";
        goto done;
    }

    *result = (uint32_t) list->num_procs;

done:
    if (list) {
        proc_stat_list_free(list);
    }
    ps_context_free(context);
    return error;
}
