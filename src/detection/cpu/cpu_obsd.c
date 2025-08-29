#include "cpu.h"
#include "common/sysctl.h"
#include "util/stringUtils.h"

#include <errno.h>
#include <sys/time.h>
#include <sys/sensors.h>

static const char* detectCPUTemp(FFCPUResult* cpu)
{
    int mib[5] = {CTL_HW, HW_SENSORS, 0, SENSOR_TEMP, 0};

    for (mib[2] = 0; mib[2] < 1024; mib[2]++)
    {
        struct sensordev sensordev;
        size_t sdlen = sizeof(struct sensordev);
        if (sysctl(mib, 3, &sensordev, &sdlen, NULL, 0) < 0)
        {
            if (errno == ENOENT)
                break;
            if (errno == ENXIO)
                continue;
            return "sysctl(sensordev) failed";
        }

        if (!ffStrStartsWith(sensordev.xname, "cpu"))
            continue;

        for (mib[4] = 0; mib[4] < sensordev.maxnumt[SENSOR_TEMP]; mib[4]++)
        {
            struct sensor sensor;
            size_t slen = sizeof(struct sensor);
            if (sysctl(mib, 5, &sensor, &slen, NULL, 0) < 0)
            {
                if (errno != ENOENT)
                    return "sysctl(sensor) failed";
                continue;
            }
            if (sensor.flags & SENSOR_FINVALID)
                continue;

            cpu->temperature = (double)(sensor.value - 273150000) / 1E6;
            return NULL;
        }
    }

    return "No sensor for CPU temp found";
}

const char *ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
{
    if (ffSysctlGetString(CTL_HW, HW_MODEL, &cpu->name))
        return "sysctl(hw.model) failed";

    cpu->coresPhysical = (uint16_t) ffSysctlGetInt(CTL_HW, HW_NCPU, 1);
    cpu->coresLogical = cpu->coresPhysical;
    cpu->coresOnline = (uint16_t) ffSysctlGetInt(CTL_HW, HW_NCPUONLINE, cpu->coresLogical);

    ffCPUDetectByCpuid(cpu);

    uint32_t cpuspeed = (uint32_t) ffSysctlGetInt(CTL_HW, HW_CPUSPEED, 0);
    if (cpuspeed > cpu->frequencyBase) cpu->frequencyBase = cpuspeed;

    cpu->temperature = FF_CPU_TEMP_UNSET;
    if (options->temp) detectCPUTemp(cpu);

    return NULL;
}
