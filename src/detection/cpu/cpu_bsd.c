#include "cpu.h"
#include "common/sysctl.h"

const char* ffDetectCPUImpl(FF_MAYBE_UNUSED const FFinstance* instance, const FFCPUOptions* options, FFCPUResult* cpu)
{
    if (ffSysctlGetString("hw.model", &cpu->name))
        return "sysctlbyname(hw.model) failed";

    cpu->coresPhysical = (uint16_t) ffSysctlGetInt("hw.ncpu", 1);
    cpu->coresLogical = cpu->coresPhysical;
    cpu->coresOnline = cpu->coresPhysical;

    cpu->frequencyMin = ffSysctlGetInt("hw.clockrate", 0) / 1000.0;
    cpu->frequencyMax = cpu->frequencyMin;

    if (options->temp)
    {
        FF_STRBUF_AUTO_DESTROY cpuTemp = ffStrbufCreate();
        if(ffSysctlGetString("hw.acpi.thermal.tz0.temperature", &cpuTemp))
            cpu->temperature = FF_CPU_TEMP_UNSET;
        else
            cpu->temperature = ffStrbufToDouble(&cpuTemp);
    }
    else
        cpu->temperature = FF_CPU_TEMP_UNSET;

    return NULL;
}
