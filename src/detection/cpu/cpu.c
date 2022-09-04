#include "cpu.h"
#include "detection/internal.h"

void ffDetectCPUImpl(FFCPUResult* cpu);

const FFCPUResult* ffDetectCPU()
{
    FF_DETECTION_INTERNAL_GUARD(FFCPUResult,
        ffDetectCPUImpl(&result);

        const char* removeStrings[] = {
            "(R)", "(r)", "(TM)", "(tm)",
            " CPU", " FPU", " APU", " Processor",
            " Dual-Core", " Quad-Core", " Six-Core", " Eight-Core", " Ten-Core",
            " 2-Core", " 4-Core", " 6-Core", " 8-Core", " 10-Core", " 12-Core", " 14-Core", " 16-Core",
            " with Radeon Graphics"
        };
        ffStrbufRemoveStringsA(&result.name, sizeof(removeStrings) / sizeof(removeStrings[0]), removeStrings);
        ffStrbufSubstrBeforeFirstC(&result.name, '@'); //Cut the speed output in the name as we append our own
        ffStrbufTrimRight(&result.name, ' '); //If we removed the @ in previous step there was most likely a space before it
    );
}
