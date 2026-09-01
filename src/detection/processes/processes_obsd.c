#include "processes.h"

#include "common/mallocHelper.h"

#include <sys/param.h>
#include <sys/sysctl.h>
#include <kvm.h>

const char* ffDetectProcesses(const FFProcessesOptions* options, FFProcessesResult* result) {
    kvm_t* kd = kvm_open(nullptr, nullptr, nullptr, KVM_NO_FILES, nullptr);
    if (!kd) {
        return "kvm_open() failed";
    }

    int count = 0;
    // KERN_PROC_ALL returns all user-level processes
    // KERN_PROC_KTHREAD returns all processes, including user-level processes (despite the name)
    const struct kinfo_proc* procs = kvm_getprocs(kd,
        (options->countKprocs ? KERN_PROC_KTHREAD : KERN_PROC_ALL) | KERN_PROC_SHOW_THREADS,
        0, sizeof(struct kinfo_proc), &count);
    if (!procs) {
        kvm_close(kd);
        return "kvm_getprocs() failed";
    }

    for (int i = 0; i < count; ++i) {
        const struct kinfo_proc* proc = &procs[i];

        ++result->threads;
        if (proc->p_tid == -1) {
            ++result->processes;
        }
    }

    kvm_close(kd);
    return nullptr;
}
