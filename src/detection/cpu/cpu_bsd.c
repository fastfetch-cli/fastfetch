#include "cpu.h"

void ffDetectCPUImpl(FFCPUResult* cpu)
{
    ffStrbufInit(&cpu->vendor);
    ffStrbufInit(&cpu->name);
    cpu->coresPhysical = 1;
    cpu->coresLogical = 1;
    cpu->coresOnline = 1;
    frequencyMin = 0.0;
    frequencyMax = 0.0;
    temperature = FF_CPU_TEMP_UNSET;
}
