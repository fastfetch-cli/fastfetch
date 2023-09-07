#include "cpu.h"
#include "common/sysctl.h"
#include "detection/temps/temps_apple.h"

static double detectCpuTemp(const FFstrbuf* cpuName)
{
    double result = 0;

    const char* error = NULL;
    if(ffStrbufStartsWithS(cpuName, "Apple M1"))
        error = ffDetectCoreTemps(FF_TEMP_CPU_M1X, &result);
    else if(ffStrbufStartsWithS(cpuName, "Apple M2"))
        error = ffDetectCoreTemps(FF_TEMP_CPU_M2X, &result);
    else // PPC?
        error = ffDetectCoreTemps(FF_TEMP_CPU_X64, &result);

    if(error)
        return FF_CPU_TEMP_UNSET;

    return result;
}

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
{
    if (ffSysctlGetString("machdep.cpu.brand_string", &cpu->name) != NULL)
        return "sysctlbyname(machdep.cpu.brand_string) failed";

    ffSysctlGetString("machdep.cpu.vendor", &cpu->vendor);
    if (cpu->vendor.length == 0 && ffStrbufStartsWithS(&cpu->name, "Apple "))
        ffStrbufAppendS(&cpu->vendor, "Apple");

    cpu->coresPhysical = (uint16_t) ffSysctlGetInt("hw.physicalcpu_max", 1);
    if(cpu->coresPhysical == 1)
        cpu->coresPhysical = (uint16_t) ffSysctlGetInt("hw.physicalcpu", 1);

    cpu->coresLogical = (uint16_t) ffSysctlGetInt("hw.logicalcpu_max", 1);
    if(cpu->coresLogical == 1)
        cpu->coresLogical = (uint16_t) ffSysctlGetInt("hw.ncpu", 1);

    cpu->coresOnline = (uint16_t) ffSysctlGetInt("hw.logicalcpu", 1);
    if(cpu->coresOnline == 1)
        cpu->coresOnline = (uint16_t) ffSysctlGetInt("hw.activecpu", 1);

    cpu->frequencyMin = ffSysctlGetInt64("hw.cpufrequency_min", 0) / 1000.0 / 1000.0 / 1000.0;
    cpu->frequencyMax = ffSysctlGetInt64("hw.cpufrequency_max", 0);
    if(cpu->frequencyMax > 0.0)
        cpu->frequencyMax /= 1000.0 * 1000.0 * 1000.0;
    else
    {
        unsigned current = 0;
        size_t size = sizeof(current);
        if (sysctl((int[]){ CTL_HW, HW_CPU_FREQ }, 2, &current, &size, NULL, 0) == 0)
            cpu->frequencyMax = (double) current / 1000.0 / 1000.0 / 1000.0;
    }

    cpu->temperature = options->temp ? detectCpuTemp(&cpu->name) : FF_CPU_TEMP_UNSET;

    return NULL;
}
