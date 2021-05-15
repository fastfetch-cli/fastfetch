#include "fastfetch.h"

#include <string.h>

#define FF_CPU_MODULE_NAME "CPU"
#define FF_CPU_NUM_FORMAT_ARGS 13

static double getGhz(const char* file)
{
    FF_STRBUF_CREATE(content);
    ffGetFileContent(file, &content);

    if(content.length == 0)
        return 0;

    uint32_t herz;
    if(sscanf(content.chars, "%u", &herz) != 1)
        return 0;

    ffStrbufDestroy(&content);

    herz /= 1000; //to MHz
    return (double) herz / 1000.0; //to GHz
}

void ffPrintCPU(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_CPU_MODULE_NAME, &instance->config.cpuKey, &instance->config.cpuFormat, FF_CPU_NUM_FORMAT_ARGS))
        return;

    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if(cpuinfo == NULL)
    {
        ffPrintError(instance, FF_CPU_MODULE_NAME, 0, &instance->config.cpuKey, &instance->config.cpuFormat, FF_CPU_NUM_FORMAT_ARGS, "fopen(\"/proc/cpuinfo\", \"r\") == NULL");
        return;
    }

    char name[256];   name[0] = '\0';
    char vendor[256]; vendor[0] = '\0';
    int physicalCores = 0;

    char* line = NULL;
    size_t len = 0;

    while(getline(&line, &len, cpuinfo) != -1)
    {
        sscanf(line, "model name%*s %256[^\n]", name);
        sscanf(line, "vendor_id%*s %256[^\n]", vendor);
        sscanf(line, "cpu cores%*s %i", &physicalCores);

        //Stop after the first CPU
        if(strstr(line, "flags") != NULL)
            break;
    }

    if(line != NULL)
        free(line);

    fclose(cpuinfo);

    double biosLimit      = getGhz("/sys/devices/system/cpu/cpu0/cpufreq/bios_limit");
    double scalingMaxFreq = getGhz("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
    double scalingMinFreq = getGhz("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
    double infoMaxFreq    = getGhz("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    double infoMinFreq    = getGhz("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq");

    int numProcsOnline = get_nprocs();
    int numProcsAvailable = get_nprocs_conf();

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
        ghz = scalingMinFreq;
    if(ghz == 0)
        ghz = infoMinFreq;

    if(
        name[0] == '\0' && //This also implies namePretty is not set
        vendor[0] == '\0' &&
        numProcs <= 1 &&
        ghz <= 0
    ) {
        ffPrintError(instance, FF_CPU_MODULE_NAME, 0, &instance->config.cpuKey, &instance->config.cpuFormat, FF_CPU_NUM_FORMAT_ARGS, "No CPU info found in /proc/cpuinfo");
        return;
    }

    FFstrbuf namePretty;
    ffStrbufInitA(&namePretty, 64);
    ffStrbufAppendS(&namePretty, name);

    const char* removeStrings[] = {
        "(R)", "(r)", "(TM)", "(tm)",
        " CPU", " FPU", " APU", " Processor",
        " Dual-Core", " Quad-Core", " Six-Core", " Eight-Core", " Ten-Core",
        " 2-Core", " 4-Core", " 6-Core", " 8-Core", " 10-Core", " 12-Core", " 14-Core", " 16-Core"
    };

    ffStrbufRemoveStringsA(&namePretty, sizeof(removeStrings) / (sizeof(removeStrings) / sizeof(removeStrings[0])), removeStrings);
    ffStrbufSubstrBeforeFirstC(&namePretty, '@'); //Cut the speed output in the name as we append our own
    ffStrbufTrimRight(&namePretty, ' '); //If we removed the @ in previous step there was most likely a space before it

    FFstrbuf cpu;
    ffStrbufInitA(&cpu, 128);

    if(namePretty.length > 0)
        ffStrbufAppend(&cpu, &namePretty);
    else if(name[0] != '\0')
        ffStrbufAppendS(&cpu, name);
    else if(vendor[0] != '\0')
    {
        ffStrbufAppendS(&cpu, vendor);
        ffStrbufAppendS(&cpu, " unknown processor");
    }
    else
        ffStrbufAppendS(&cpu, " unknown processor");

    if(numProcs > 1)
        ffStrbufAppendF(&cpu, " (%i)", numProcs);

    if(ghz > 0)
        ffStrbufAppendF(&cpu, " @ %.9gGHz", ghz);

    ffPrintAndSaveToCache(instance, FF_CPU_MODULE_NAME, &instance->config.cpuKey, &cpu, &instance->config.cpuFormat, FF_CPU_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRING, name},
        {FF_FORMAT_ARG_TYPE_STRBUF, &namePretty},
        {FF_FORMAT_ARG_TYPE_STRING, vendor},
        {FF_FORMAT_ARG_TYPE_INT, &numProcsOnline},
        {FF_FORMAT_ARG_TYPE_INT, &numProcsAvailable},
        {FF_FORMAT_ARG_TYPE_INT, &physicalCores},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &biosLimit},
        {FF_FORMAT_ARG_TYPE_INT, &numProcs},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &scalingMaxFreq},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &scalingMinFreq},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &infoMaxFreq},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &infoMinFreq},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &ghz}
    });

    ffStrbufDestroy(&namePretty);
    ffStrbufDestroy(&cpu);
}
