#include "cpu.h"
#include "common/sysctl.h"

#include <sys/time.h>
#include <sys/sensors.h>

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
{
    if (ffSysctlGetString(CTL_HW, HW_MODEL, &cpu->name))
        return "sysctl(hw.model) failed";

    cpu->coresPhysical = (uint16_t) ffSysctlGetInt(CTL_HW, HW_NCPU, 1);
    cpu->coresLogical = cpu->coresPhysical;
    cpu->coresOnline = (uint16_t) ffSysctlGetInt(CTL_HW, HW_NCPUONLINE, cpu->coresLogical);

    ffCPUDetectSpeedByCpuid(cpu);

    uint32_t cpuspeed = (uint32_t) ffSysctlGetInt(CTL_HW, HW_CPUSPEED, 0);
    if (cpuspeed > cpu->frequencyBase) cpu->frequencyBase = cpuspeed;

    cpu->temperature = FF_CPU_TEMP_UNSET;
    if (options->temp)
    {
        struct sensor sensors;
        size_t size = sizeof(sensors);
        if (sysctl((int[]) {CTL_HW, HW_SENSORS, 0, SENSOR_TEMP, 0}, 5, &sensors, &size, NULL, 0) == 0)
            cpu->temperature = (double) (sensors.value - 273150000) / 1E6;
    }

    return NULL;
}
