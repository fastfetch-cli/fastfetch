#include "cpucache.h"
#include "util/mallocHelper.h"

#include <windows.h>

const char* ffDetectCPUCache(FFCPUCacheResult* result)
{
    DWORD length = 0;
    GetLogicalProcessorInformationEx(RelationCache, NULL, &length);
    if (length == 0)
        return "GetLogicalProcessorInformationEx(RelationCache, NULL, &length) failed";

    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* FF_AUTO_FREE
        pProcessorInfo = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)malloc(length);

    if (!pProcessorInfo || !GetLogicalProcessorInformationEx(RelationCache, pProcessorInfo, &length))
        return "GetLogicalProcessorInformationEx(RelationCache, pProcessorInfo, &length) failed";

    for(
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* ptr = pProcessorInfo;
        (uint8_t*)ptr < ((uint8_t*)pProcessorInfo) + length;
        ptr = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(((uint8_t*)ptr) + ptr->Size)
    )
    {
        if(__builtin_expect(ptr->Relationship == RelationCache && ptr->Cache.Level > 0 && ptr->Cache.Level <= 4, true))
        {
            FFCPUCacheType cacheType = 0;
            switch (ptr->Cache.Type)
            {
            case CacheUnified: cacheType = FF_CPU_CACHE_TYPE_UNIFIED; break;
            case CacheInstruction: cacheType = FF_CPU_CACHE_TYPE_INSTRUCTION; break;
            case CacheData: cacheType = FF_CPU_CACHE_TYPE_DATA; break;
            case CacheTrace: cacheType = FF_CPU_CACHE_TYPE_TRACE; break;
            default: __builtin_unreachable(); break;
            }
            ffCPUCacheAddItem(result, ptr->Cache.Level, ptr->Cache.CacheSize, ptr->Cache.LineSize, cacheType);
        }
    }

    return NULL;
}
