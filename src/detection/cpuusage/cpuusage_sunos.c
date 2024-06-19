#include "fastfetch.h"
#include "detection/cpuusage/cpuusage.h"

#include <kstat.h>
#include <sys/sysinfo.h>

static inline void kstatFreeWrap(kstat_ctl_t** pkc)
{
    assert(pkc);
    if (*pkc)
        kstat_close(*pkc);
}

const char* ffGetCpuUsageInfo(FFlist* cpuTimes)
{
    __attribute__((__cleanup__(kstatFreeWrap))) kstat_ctl_t* kc = kstat_open();
    if (!kc)
        return "kstat_open() failed";

    for (int i = 0;; ++i)
    {
        kstat_t* ks = kstat_lookup(kc, "cpu_stat", i, NULL);

        cpu_stat_t cs;
        if (!ks || kstat_read(kc, ks, &cs) < 0)
            break;

        uint64_t inUse = cs.cpu_sysinfo.cpu[CPU_USER] + cs.cpu_sysinfo.cpu[CPU_KERNEL];
        uint64_t total = inUse + cs.cpu_sysinfo.cpu[CPU_IDLE] + cs.cpu_sysinfo.cpu[CPU_WAIT];

        FFCpuUsageInfo* info = (FFCpuUsageInfo*) ffListAdd(cpuTimes);
        *info = (FFCpuUsageInfo) {
            .inUseAll = inUse,
            .totalAll = total,
        };
    }
    return NULL;
}
