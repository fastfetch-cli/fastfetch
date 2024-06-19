#include "detection/cpuusage/cpuusage.h"
#include "util/mallocHelper.h"

#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <stdlib.h>

const char* ffGetCpuUsageInfo(FFlist* cpuTimes)
{
    size_t neededLength = 0;
    if(sysctlbyname("kern.cp_times", NULL, &neededLength, NULL, 0) != 0)
        return "sysctlbyname(kern.cp_times, NULL) failed";

    uint32_t coreCount = (uint32_t) (neededLength / (CPUSTATES * sizeof(uint64_t)));
    assert(coreCount > 0);

    FF_AUTO_FREE uint64_t (*cpTimes)[CPUSTATES] = malloc(neededLength);
    if(sysctlbyname("kern.cp_times", cpTimes, &neededLength, NULL, 0) != 0)
        return "sysctlbyname(kern.cp_times, cpTime) failed";

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
