#include "temps_bsd.h"
#include "common/sysctl.h"

const char* ffDetectCpuTemp(double* current)
{
    int temp = ffSysctlGetInt("dev.cpu.0.temperature", -999999);
    if (temp == -999999)
        return "ffSysctlGetInt(\"dev.cpu.0.temperature\") failed";

    // In tenth of degrees Kelvin
    *current = (double) temp / 10 - 273.15;
    return NULL;
}

const char* ffDetectThermalTemp(double* current)
{
    int temp = ffSysctlGetInt("hw.acpi.thermal.tz0.temperature", -999999);
    if (temp == -999999)
        return "ffSysctlGetInt(\"hw.acpi.thermal.tz0.temperature\") failed";

    // In tenth of degrees Kelvin
    *current = (double) temp / 10 - 273.15;
    return NULL;
}
