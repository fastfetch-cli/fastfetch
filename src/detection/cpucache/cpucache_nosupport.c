#include "cpucache.h"
#include "common/sysctl.h"

const char* ffDetectCPUCache(FF_MAYBE_UNUSED FFCPUCacheResult* result)
{
    return "Not supported on this platform";
}
