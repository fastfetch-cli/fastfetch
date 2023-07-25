#include "cpu.h"
#include "detection/internal.h"

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu);

const char* ffDetectCPU(const FFCPUOptions* options, FFCPUResult* cpu)
{
    const char* error = ffDetectCPUImpl(options, cpu);
    if (error) return error;

    const char* removeStrings[] = {
        " CPU", " FPU", " APU", " Processor",
        " Dual-Core", " Quad-Core", " Six-Core", " Eight-Core", " Ten-Core",
        " 2-Core", " 4-Core", " 6-Core", " 8-Core", " 10-Core", " 12-Core", " 14-Core", " 16-Core",
        " with Radeon Graphics"
    };
    ffStrbufRemoveStrings(&cpu->name, sizeof(removeStrings) / sizeof(removeStrings[0]), removeStrings);
    ffStrbufSubstrBeforeFirstC(&cpu->name, '@'); //Cut the speed output in the name as we append our own
    ffStrbufTrimRight(&cpu->name, ' '); //If we removed the @ in previous step there was most likely a space before it
    return NULL;
}
