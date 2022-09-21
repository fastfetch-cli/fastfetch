#include "cpu.h"
#include "common/io.h"
#include "common/properties.h"
#include "detection/temps.h"

#include <stdlib.h>
#include <unistd.h>

static void parseCpuInfo(FFCPUResult* cpu, FFstrbuf* physicalCoresBuffer)
{
    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if(cpuinfo == NULL)
        return;

    char* line = NULL;
    size_t len = 0;

    while(getline(&line, &len, cpuinfo) != -1)
    {
        //Stop after the first CPU
        if(cpu->name.length > 0 && (*line == '\0' || *line == '\n'))
            break;

        (void)(
            ffParsePropLine(line, "model name :", &cpu->name) ||
            ffParsePropLine(line, "vendor_id :", &cpu->vendor) ||
            ffParsePropLine(line, "cpu cores :", physicalCoresBuffer) ||
            (cpu->name.length == 0 && ffParsePropLine(line, "Hardware :", &cpu->name)) //For Android devices
        );
    }

    if(line != NULL)
        free(line);

    fclose(cpuinfo);
}

static double getGHz(const char* file)
{
    FFstrbuf content;
    ffStrbufInit(&content);
    ffReadFileBuffer(file, &content);
    double herz = ffStrbufToDouble(&content);
    ffStrbufDestroy(&content);

    //ffStrbufToDouble failed
    if(herz != herz)
        return 0;

    herz /= 1000.0; //to MHz
    return herz / 1000.0; //to GHz
}

static double getFrequency(const char* info, const char* scaling)
{
    double frequency = getGHz(info);
    if(frequency > 0.0)
        return frequency;

    return getGHz(scaling);
}

static double detectCPUTemp()
{
    const FFTempsResult* temps = ffDetectTemps();

    for(uint32_t i = 0; i < temps->values.length; i++)
    {
        FFTempValue* value = ffListGet(&temps->values, i);

        if(
            ffStrbufFirstIndexS(&value->name, "cpu") < value->name.length ||
            ffStrbufCompS(&value->name, "k10temp") == 0 ||
            ffStrbufCompS(&value->name, "coretemp") == 0
        ) return value->value;
    }

    return FF_CPU_TEMP_UNSET;
}

void ffDetectCPUImpl(FFCPUResult* cpu, bool cached)
{
    cpu->temperature = detectCPUTemp();
    if(cached)
        return;

    FFstrbuf physicalCoresBuffer;
    ffStrbufInit(&physicalCoresBuffer);

    parseCpuInfo(cpu, &physicalCoresBuffer);

    cpu->coresPhysical = ffStrbufToUInt16(&physicalCoresBuffer, 1);

    #ifdef FF_HAVE_SYSINFO_H
        cpu->coresLogical = (uint16_t) get_nprocs_conf();
        cpu->coresOnline = (uint16_t) get_nprocs();
    #else
        cpu->coresLogical = 1;
        cpu->coresOnline = 1;
    #endif

    #define BP "/sys/devices/system/cpu/cpufreq/policy0/"
    cpu->frequencyMin = getFrequency(BP"cpuinfo_min_freq", BP"scaling_min_freq");
    cpu->frequencyMax = getFrequency(BP"cpuinfo_max_freq", BP"scaling_max_freq");

    ffStrbufDestroy(&physicalCoresBuffer);
}
