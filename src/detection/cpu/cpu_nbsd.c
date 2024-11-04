#include "cpu.h"
#include "common/sysctl.h"
#include <sys/sysctl.h>

static const char* detectCpuTemp(double* current)
{
    int temp = ffSysctlGetInt("dev.cpu.0.temperature", -999999);
    if (temp == -999999)
        return "ffSysctlGetInt(\"dev.cpu.0.temperature\") failed";

    // In tenth of degrees Kelvin
    *current = (double) temp / 10 - 273.15;
    return NULL;
}

static const char* detectThermalTemp(double* current)
{
    int temp = ffSysctlGetInt("hw.acpi.thermal.tz0.temperature", -999999);
    if (temp == -999999)
        return "ffSysctlGetInt(\"hw.acpi.thermal.tz0.temperature\") failed";

    // In tenth of degrees Kelvin
    *current = (double) temp / 10 - 273.15;
    return NULL;
}

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
{
    if (ffSysctlGetString("machdep.cpu_brand", &cpu->name) != NULL)
    {
        if (ffSysctlGetString("machdep.dmi.processor-version", &cpu->name) != NULL)
            return "sysctlbyname(machdep.cpu_brand) failed";
    }

    cpu->coresPhysical = (uint16_t) ffSysctlGetInt("hw.ncpu", 1);
    cpu->coresLogical = cpu->coresPhysical;
    cpu->coresOnline = (uint16_t) ffSysctlGetInt("hw.ncpuonline", cpu->coresLogical);

    ffCPUDetectSpeedByCpuid(cpu);

    {
        struct clockinfo info;
        size_t length = sizeof(info);
        if (sysctl((int[]) {CTL_KERN, KERN_CLOCKRATE}, 2, &info, &length, NULL, 0) == 0)
            cpu->frequencyBase = (uint32_t) info.hz / 1000;
    }

    cpu->temperature = FF_CPU_TEMP_UNSET;

    if (options->temp)
    {
        if (detectCpuTemp(&cpu->temperature) != NULL)
            detectThermalTemp(&cpu->temperature);
    }

    return NULL;
}
