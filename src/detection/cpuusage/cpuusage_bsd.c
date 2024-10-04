#include "detection/cpuusage/cpuusage.h"
#include "util/mallocHelper.h"

#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <stdlib.h>

#ifdef __OpenBSD__
    #include <sys/sched.h>
#endif

const char* ffGetCpuUsageInfo(FFlist* cpuTimes)
{
    size_t neededLength = 0;
#ifdef __OpenBSD__
    int ctls[] = {CTL_KERN, KERN_CPTIME};
    if (sysctl(ctls, 2, NULL, &neededLength, NULL, 0) != 0)
        return "sysctl({CTL_KERN, KERN_CPTIME}, 2, NULL) failed";
#else
    if(sysctlbyname("kern.cp_times", NULL, &neededLength, NULL, 0) != 0)
        return "sysctlbyname(kern.cp_times, NULL) failed";
#endif

    uint32_t coreCount = (uint32_t) (neededLength / (CPUSTATES * sizeof(uint64_t)));
    assert(coreCount > 0);

    FF_AUTO_FREE uint64_t (*cpTimes)[CPUSTATES] = malloc(neededLength);

#ifdef __OpenBSD__
    if (sysctl(ctls, 2, cpTimes, &neededLength, NULL, 0) != 0)
        return "sysctl({CTL_KERN, KERN_CPTIME}, 2, NULL) failed";
#else
    if(sysctlbyname("kern.cp_times", cpTimes, &neededLength, NULL, 0) != 0)
        return "sysctlbyname(kern.cp_times, cpTime) failed";
#endif

    for (uint32_t i = 0; i < coreCount; ++i)
    {
        uint64_t* cpTime = cpTimes[i];
        uint64_t inUse = cpTime[CP_USER] + cpTime[CP_NICE] + cpTime[CP_SYS];
        uint64_t total = inUse + cpTime[CP_INTR] + cpTime[CP_IDLE];

        FFCpuUsageInfo* info = (FFCpuUsageInfo*) ffListAdd(cpuTimes);
        *info = (FFCpuUsageInfo) {
            .inUseAll = inUse,
            .totalAll = total,
        };
    }

    return NULL;
}
