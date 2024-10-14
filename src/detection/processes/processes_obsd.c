#include "processes.h"

#include <sys/param.h>
#include <sys/sysctl.h>
#include <kvm.h>

const char* ffDetectProcesses(uint32_t* result)
{
    kvm_t* kd = kvm_open(NULL, NULL, NULL, KVM_NO_FILES, NULL);
    const void* ret = kvm_getprocs(kd, KERN_PROC_ALL, 0, 1, result);
    kvm_close(kd);
    if (!ret) return "kvm_getprocs() failed";
    return NULL;
}
