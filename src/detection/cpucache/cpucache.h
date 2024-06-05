#pragma once

#include "fastfetch.h"

typedef enum FFCPUCacheType
{
    FF_CPU_CACHE_TYPE_UNIFIED = 0,
    FF_CPU_CACHE_TYPE_INSTRUCTION = 1,
    FF_CPU_CACHE_TYPE_DATA = 2,
    FF_CPU_CACHE_TYPE_TRACE = 3,
} FFCPUCacheType;

typedef struct FFCPUCache
{
    uint32_t size;
    uint32_t num;
    FFCPUCacheType type;
} FFCPUCache;

typedef struct FFCPUCacheResult
{
    FFlist caches[4]; // L1, L2, L3, L4(?)
} FFCPUCacheResult;

const char* ffDetectCPUCache(FFCPUCacheResult* result);

static inline void ffCPUCacheAddItem(FFCPUCacheResult* result, uint32_t level, uint32_t size, FFCPUCacheType type)
{
    FFlist* cacheLevel = &result->caches[level - 1];

    FF_LIST_FOR_EACH(FFCPUCache, item, *cacheLevel)
    {
        if (item->type == type && item->size == size)
        {
            item->num++;
            return;
        }
    }
    *(FFCPUCache*) ffListAdd(cacheLevel) = (FFCPUCache) {
        .size = size,
        .num = 1,
        .type = type,
    };
}
