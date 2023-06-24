#include "cpu.h"
#include "common/sysctl.h"
#include "detection/temps/temps_apple.h"

static double getFrequency(const char* propName)
{
    double herz = (double) ffSysctlGetInt64(propName, 0);
    if(herz <= 0.0)
        return herz;

    herz /= 1000.0; //to KHz
    herz /= 1000.0; //to MHz
    return herz / 1000.0; //to GHz
}

static double detectCpuTemp(const FFstrbuf* cpuName)
{
    FF_LIST_AUTO_DESTROY temps = ffListCreate(sizeof(FFTempValue));

    if(ffStrbufStartsWithS(cpuName, "Apple M1"))
        ffDetectCoreTemps(FF_TEMP_CPU_M1X, &temps);
    else if(ffStrbufStartsWithS(cpuName, "Apple M2"))
        ffDetectCoreTemps(FF_TEMP_CPU_M2X, &temps);
    else //TODO: PPC?
        ffDetectCoreTemps(FF_TEMP_CPU_X64, &temps);

    if(temps.length == 0)
        return FF_CPU_TEMP_UNSET;

    double result = 0;
    for(uint32_t i = 0; i < temps.length; ++i)
    {
        FFTempValue* tempValue = (FFTempValue*)ffListGet(&temps, i);
        result += tempValue->value;
        //TODO: do we really need this?
        ffStrbufDestroy(&tempValue->name);
        ffStrbufDestroy(&tempValue->deviceClass);
    }
    result /= temps.length;
    return result;
}

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
{
    if (ffSysctlGetString("machdep.cpu.brand_string", &cpu->name) != NULL)
        return "sysctlbyname(machdep.cpu.brand_string) failed";
    ffSysctlGetString("machdep.cpu.vendor", &cpu->vendor);

    cpu->coresPhysical = (uint16_t) ffSysctlGetInt("hw.physicalcpu_max", 1);
    if(cpu->coresPhysical == 1)
        cpu->coresPhysical = (uint16_t) ffSysctlGetInt("hw.physicalcpu", 1);

    cpu->coresLogical = (uint16_t) ffSysctlGetInt("hw.logicalcpu_max", 1);
    if(cpu->coresLogical == 1)
        cpu->coresLogical = (uint16_t) ffSysctlGetInt("hw.ncpu", 1);

    cpu->coresOnline = (uint16_t) ffSysctlGetInt("hw.logicalcpu", 1);
    if(cpu->coresOnline == 1)
        cpu->coresOnline = (uint16_t) ffSysctlGetInt("hw.activecpu", 1);

    cpu->frequencyMin = getFrequency("hw.cpufrequency_min");
    cpu->frequencyMax = getFrequency("hw.cpufrequency_max");
    if(cpu->frequencyMax == 0.0)
        cpu->frequencyMax = getFrequency("hw.cpufrequency");

    if (options->temp)
        cpu->temperature = detectCpuTemp(&cpu->name);
    else
        cpu->temperature = FF_CPU_TEMP_UNSET;

    return NULL;
}
