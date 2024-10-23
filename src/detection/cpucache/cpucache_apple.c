#include "cpucache.h"
#include "common/sysctl.h"
#include "util/stringUtils.h"

const char* ffDetectCPUCache(FFCPUCacheResult* result)
{
    // https://developer.apple.com/documentation/kernel/1387446-sysctlbyname/determining_system_capabilities#3901385
    uint32_t nPerfLevels = (uint32_t) ffSysctlGetInt("hw.nperflevels", 0);
    if (nPerfLevels <= 0) return "sysctl(hw.nperflevels) failed";

    // macOS provides the global system cache line size
    uint32_t lineSize = (uint32_t) ffSysctlGetInt("hw.cachelinesize", 0);

    char sysctlKey[128] = "hw.perflevelN.";
    char* pNum = sysctlKey + strlen("hw.perflevel");
    char* pSubkey = sysctlKey + strlen("hw.perflevelN.");
    const size_t lenLeft = ARRAY_SIZE(sysctlKey) - strlen("hw.perflevelN.");

    for (uint32_t i = 0; i < nPerfLevels; ++i)
    {
        *pNum = (char) ('0' + i);

        ffStrCopyN(pSubkey, "physicalcpu", lenLeft);
        uint32_t ncpu = (uint32_t) ffSysctlGetInt(sysctlKey, 0);
        if (ncpu <= 0) continue;

        ffStrCopyN(pSubkey, "l1icachesize", lenLeft);
        uint32_t size = (uint32_t) ffSysctlGetInt(sysctlKey, 0);
        if (size)
            ffCPUCacheAddItem(result, 1, size, lineSize, FF_CPU_CACHE_TYPE_INSTRUCTION)->num = ncpu;

        ffStrCopyN(pSubkey, "l1dcachesize", lenLeft);
        size = (uint32_t) ffSysctlGetInt(sysctlKey, 0);
        if (size)
            ffCPUCacheAddItem(result, 1, size, lineSize, FF_CPU_CACHE_TYPE_DATA)->num = ncpu;

        ffStrCopyN(pSubkey, "l2cachesize", lenLeft);
        size = (uint32_t) ffSysctlGetInt(sysctlKey, 0);
        if (size)
        {
            ffStrCopyN(pSubkey, "cpusperl2", lenLeft);
            uint32_t cpuSper = (uint32_t) ffSysctlGetInt(sysctlKey, 0);
            if (cpuSper)
                ffCPUCacheAddItem(result, 2, size, lineSize, FF_CPU_CACHE_TYPE_UNIFIED)->num = ncpu / cpuSper;
        }

        ffStrCopyN(pSubkey, "l3cachesize", lenLeft);
        size = (uint32_t) ffSysctlGetInt(sysctlKey, 0);
        if (size)
        {
            ffStrCopyN(pSubkey, "cpusperl3", lenLeft);
            uint32_t cpuSper = (uint32_t) ffSysctlGetInt(sysctlKey, 0);
            if (cpuSper)
                ffCPUCacheAddItem(result, 3, size, lineSize, FF_CPU_CACHE_TYPE_UNIFIED)->num = ncpu / cpuSper;
        }
    }

    return NULL;
}
