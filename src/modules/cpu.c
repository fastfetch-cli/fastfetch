#include "fastfetch.h"

#include <string.h>
#include <errno.h>

#define FF_CPU_MODULE_NAME "CPU"
#define FF_CPU_NUM_FORMAT_ARGS 15

static double getGhz(const char* policyFile, const char* cpuFile)
{
    FFstrbuf content;
    ffStrbufInit(&content);

    ffGetFileContent(policyFile, &content);
    if(content.length == 0)
        ffGetFileContent(cpuFile, &content);

    double herz = ffStrbufToDouble(&content);

    ffStrbufDestroy(&content);

    //ffStrbufToDouble failed
    if(herz != herz)
        return 0;

    herz /= 1000.0; //to MHz
    return herz / 1000.0; //to GHz
}

static double detectCPUTemp(const FFinstance* instance)
{
    const FFTempsResult *temps = ffDetectTemps(instance);

    for(uint32_t i = 0; i < temps->values.length; i++)
    {
        FFTempValue* value = ffListGet(&temps->values, i);

        if(
            ffStrbufFirstIndexS(&value->name, "cpu") < value->name.length ||
            ffStrbufCompS(&value->name, "k10temp") == 0 ||
            ffStrbufCompS(&value->name, "coretemp") == 0
        ) return value->value;
    }

    return 0.0 / 0.0; //NaN
}

void ffPrintCPU(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_CPU_MODULE_NAME, &instance->config.cpu, FF_CPU_NUM_FORMAT_ARGS))
        return;

    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if(cpuinfo == NULL)
    {
        ffPrintError(instance, FF_CPU_MODULE_NAME, 0, &instance->config.cpu, "fopen(\"""/proc/cpuinfo\", \"r\") == NULL");
        return;
    }

    FFstrbuf name;
    ffStrbufInitA(&name, 64);

    FFstrbuf vendor;
    ffStrbufInitA(&vendor, 64);

    FFstrbuf physicalCoresString;
    ffStrbufInit(&physicalCoresString);

    FFstrbuf procGhzString;
    ffStrbufInit(&procGhzString);

    char* line = NULL;
    size_t len = 0;

    while(getline(&line, &len, cpuinfo) != -1)
    {
        //Stop after the first CPU
        if(name.length > 0 && (*line == '\0' || *line == '\n'))
            break;

        (void)(
            ffParsePropLine(line, "model name :", &name) ||
            ffParsePropLine(line, "vendor_id :", &vendor) ||
            ffParsePropLine(line, "cpu cores :", &physicalCoresString) ||
            ffParsePropLine(line, "cpu MHz :", &procGhzString) ||
            (name.length == 0 && ffParsePropLine(line, "Hardware :", &name)) //For Android devices
        );
    }

    if(line != NULL)
        free(line);

    fclose(cpuinfo);

    double procGhz = ffStrbufToDouble(&procGhzString);
    if(procGhz != procGhz)
        procGhz = 0; //NaN
    else
        procGhz /= 1000.0; //To GHz

    ffStrbufDestroy(&procGhzString);

    double biosLimit      = getGhz("/sys/devices/system/cpu/cpufreq/policy0/bios_limit",       "/sys/devices/system/cpu/cpu0/cpufreq/bios_limit");
    double scalingMaxFreq = getGhz("/sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
    double scalingMinFreq = getGhz("/sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
    double infoMaxFreq    = getGhz("/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_max_freq", "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    double infoMinFreq    = getGhz("/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_min_freq", "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq");

    int numProcsOnline = get_nprocs();
    int numProcsAvailable = get_nprocs_conf();

    int physicalCores = 1;
    sscanf(physicalCoresString.chars, "%i", &physicalCores);
    ffStrbufDestroy(&physicalCoresString);

    //The current get_nprocs* returns 1 on failure. It also makes no sense to have a (1) as count
    int numProcs = numProcsOnline;
    if(numProcs <= 1)
        numProcs = numProcsAvailable;
    if(numProcs <= 1)
        numProcs = physicalCores;

    double ghz = biosLimit;
    if(ghz == 0)
        ghz = scalingMaxFreq;
    if(ghz == 0)
        ghz = infoMaxFreq;
    if(ghz == 0)
        ghz = procGhz;
    if(ghz == 0)
        ghz = scalingMinFreq;
    if(ghz == 0)
        ghz = infoMinFreq;

    if(
        name.length == 0 &&
        vendor.length == 0 &&
        numProcs <= 1 &&
        ghz <= 0
    ) {
        ffStrbufDestroy(&name);
        ffStrbufDestroy(&vendor);
        ffPrintError(instance, FF_CPU_MODULE_NAME, 0, &instance->config.cpu, "No CPU info found in /proc/cpuinfo");
        return;
    }

    FFstrbuf namePretty;
    ffStrbufInitA(&namePretty, 64);
    ffStrbufAppend(&namePretty, &name);

    const char* removeStrings[] = {
        "(R)", "(r)", "(TM)", "(tm)",
        " CPU", " FPU", " APU", " Processor",
        " Dual-Core", " Quad-Core", " Six-Core", " Eight-Core", " Ten-Core",
        " 2-Core", " 4-Core", " 6-Core", " 8-Core", " 10-Core", " 12-Core", " 14-Core", " 16-Core"
    };

    ffStrbufRemoveStringsA(&namePretty, sizeof(removeStrings) / sizeof(removeStrings[0]), removeStrings);
    ffStrbufSubstrBeforeFirstC(&namePretty, '@'); //Cut the speed output in the name as we append our own
    ffStrbufTrimRight(&namePretty, ' '); //If we removed the @ in previous step there was most likely a space before it

    double cpuTemp;
    if(instance->config.cpu.outputFormat.length > 0)
        cpuTemp = detectCPUTemp(instance);

    FFstrbuf cpu;
    ffStrbufInitA(&cpu, 128);

    if(namePretty.length > 0)
        ffStrbufAppend(&cpu, &namePretty);
    else if(name.length > 0)
        ffStrbufAppend(&cpu, &name);
    else if(vendor.length > 0)
    {
        ffStrbufAppend(&cpu, &vendor);
        ffStrbufAppendS(&cpu, " CPU");
    }
    else
        ffStrbufAppendS(&cpu, "CPU");

    if(numProcs > 1)
        ffStrbufAppendF(&cpu, " (%i)", numProcs);

    if(ghz > 0)
        ffStrbufAppendF(&cpu, " @ %.9gGHz", ghz);

    ffPrintAndWriteToCache(instance, FF_CPU_MODULE_NAME, &instance->config.cpu, &cpu, FF_CPU_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRBUF, &name},
        {FF_FORMAT_ARG_TYPE_STRBUF, &namePretty},
        {FF_FORMAT_ARG_TYPE_STRBUF, &vendor},
        {FF_FORMAT_ARG_TYPE_INT, &numProcsOnline},
        {FF_FORMAT_ARG_TYPE_INT, &numProcsAvailable},
        {FF_FORMAT_ARG_TYPE_INT, &physicalCores},
        {FF_FORMAT_ARG_TYPE_INT, &numProcs},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &cpuTemp},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &biosLimit},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &scalingMaxFreq},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &scalingMinFreq},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &infoMaxFreq},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &infoMinFreq},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &procGhz},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &ghz}
    });

    ffStrbufDestroy(&cpu);
    ffStrbufDestroy(&namePretty);
    ffStrbufDestroy(&name);
    ffStrbufDestroy(&vendor);
}
