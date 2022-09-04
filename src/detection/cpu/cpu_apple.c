#include "cpu.h"

void ffDetectCPUImpl(FFCPUResult* cpu)
{
    ffStrbufInit(&cpu->name);
    ffStrbufInit(&cpu->vendor);

    cpu->coresPhysical = 0;
    cpu->coresLogical = 0;
    cpu->coresOnline = 0;

    cpu->frequencyCurrent = 0.0;
    cpu->frequencyMin = 0.0;
    cpu->frequencyMax = 0.0;

    cpu->temperature = FF_CPU_TEMP_UNKNOWN;
}
