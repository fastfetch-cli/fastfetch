#include "cpu.h"
#include "common/sysctl.h"

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
{
    if (ffSysctlGetString(CTL_HW, HW_MODEL, &cpu->name))
        return "sysctl(hw.model) failed";

    cpu->coresPhysical = (uint16_t) ffSysctlGetInt(CTL_HW, HW_NCPU, 1);
    cpu->coresLogical = cpu->coresPhysical;
    cpu->coresOnline = (uint16_t) ffSysctlGetInt(CTL_HW, HW_NCPUONLINE, cpu->coresLogical);

    ffCPUDetectSpeedByCpuid(cpu);

    cpu->frequencyBase = (uint32_t) ffSysctlGetInt(CTL_HW, HW_CPUSPEED, 0);
    cpu->temperature = FF_CPU_TEMP_UNSET; // HW_SENSORS?

    return NULL;
}
