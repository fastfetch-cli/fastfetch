#include "fastfetch.h"
#include "detection/cpuusage/cpuusage.h"
#include "util/mallocHelper.h"

#include <OS.h>

const char* ffGetCpuUsageInfo(FFlist* cpuTimes)
{
    system_info sysInfo;
    if (get_system_info(&sysInfo) != B_OK)
        return "get_system_info() failed";

    FF_AUTO_FREE cpu_info* cpuInfo = malloc(sizeof(*cpuInfo) * sysInfo.cpu_count);
    if (get_cpu_info(0, sysInfo.cpu_count, cpuInfo) != B_OK)
        return "get_cpu_info() failed";

    uint64_t uptime = (uint64_t) system_time();

    for (uint32_t i = 0; i < sysInfo.cpu_count; ++i)
    {
        FFCpuUsageInfo* info = (FFCpuUsageInfo*) ffListAdd(cpuTimes);
        info->inUseAll = (uint64_t) cpuInfo[i].active_time;
        info->totalAll = uptime;
    }

    return NULL;
}
