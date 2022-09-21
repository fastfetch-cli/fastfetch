#include "cpu.h"
#include "common/caching.h"
#include "detection/internal.h"

#define FF_CPU_CACHE_NAME "cpu"

void ffDetectCPUImpl(FFCPUResult* cpu, bool cached);

static bool cacheCallback(FFCPUResult* cpu, FFCache* cache, FFCacheMethodStrbuf strbufMethod, FFCacheMethodData dataMethod)
{
    return
        strbufMethod(cache, &cpu->vendor) &&
        strbufMethod(cache, &cpu->name) &&
        dataMethod(cache, sizeof(cpu->coresPhysical), &cpu->coresPhysical) &&
        dataMethod(cache, sizeof(cpu->coresLogical), &cpu->coresLogical) &&
        dataMethod(cache, sizeof(cpu->coresOnline), &cpu->coresOnline) &&
        dataMethod(cache, sizeof(cpu->frequencyMin), &cpu->frequencyMin) &&
        dataMethod(cache, sizeof(cpu->frequencyMax), &cpu->frequencyMax);
}

static void detectCPU(const FFinstance* instance, FFCPUResult* cpu)
{
    ffStrbufInit(&cpu->name);
    ffStrbufInit(&cpu->vendor);

    bool cached = ffCacheRead(instance, cpu, FF_CPU_CACHE_NAME, (FFCacheMethodCallback) cacheCallback);

    ffDetectCPUImpl(cpu, cached);

    if(cached)
        return;

    const char* removeStrings[] = {
        "(R)", "(r)", "(TM)", "(tm)",
        " CPU", " FPU", " APU", " Processor",
        " Dual-Core", " Quad-Core", " Six-Core", " Eight-Core", " Ten-Core",
        " 2-Core", " 4-Core", " 6-Core", " 8-Core", " 10-Core", " 12-Core", " 14-Core", " 16-Core",
        " with Radeon Graphics"
    };
    ffStrbufRemoveStringsA(&cpu->name, sizeof(removeStrings) / sizeof(removeStrings[0]), removeStrings);
    ffStrbufSubstrBeforeFirstC(&cpu->name, '@'); //Cut the speed output in the name as we append our own
    ffStrbufTrimRight(&cpu->name, ' '); //If we removed the @ in previous step there was most likely a space before it

    ffCacheWrite(instance, cpu, FF_CPU_CACHE_NAME, (FFCacheMethodCallback) cacheCallback);
}

const FFCPUResult* ffDetectCPU(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFCPUResult,
        detectCPU(instance, &result);
    );
}
