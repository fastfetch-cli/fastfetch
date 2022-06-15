#include "fastfetch.h"

#include <string.h>
#include <errno.h>

#define FF_CPU_MODULE_NAME "CPU"
#define FF_CPU_NUM_FORMAT_ARGS 15

// TODO: possibly add a config option for this
char *FF_CPU_THERMAL_ZONES[] = {
    "x86_pkg_temp",  // x86 CPU package temperature
    "cpu-thermal"    // Raspberry Pi CPU temperature (possibly other ARM platforms aswell)
};

static double parseHz(FFstrbuf* content)
{
    if(content->length == 0)
        return 0;

    double herz;
    if(sscanf(content->chars, "%lf", &herz) != 1)
        return 0;

    return herz;
}

static double getGhz(const char* policyFile, const char* cpuFile)
{
    FFstrbuf content;
    ffStrbufInit(&content);

    ffGetFileContent(policyFile, &content);
    if(content.length == 0)
        ffGetFileContent(cpuFile, &content);

    double herz = parseHz(&content);

    ffStrbufDestroy(&content);

    herz /= 1000.0; //to MHz
    return herz / 1000.0; //to GHz
}

void ffPrintCPU(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_CPU_MODULE_NAME, &instance->config.cpuKey, &instance->config.cpuFormat, FF_CPU_NUM_FORMAT_ARGS))
        return;

    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if(cpuinfo == NULL)
    {
        ffPrintError(instance, FF_CPU_MODULE_NAME, 0, &instance->config.cpuKey, &instance->config.cpuFormat, FF_CPU_NUM_FORMAT_ARGS, "fopen(\"""/proc/cpuinfo\", \"r\") == NULL");
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

    double procGhz = parseHz(&procGhzString) / 1000.0; //to GHz
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

    double cpuPackageTemp = __DBL_MAX__;
    // enumerate thermal zones until we either find the x86_pkg_temp zone or we run out of zones.
    // NOTE: This should generally run just a couple times (~10 iterations), but maybe we should put this behind allowSlowOperations?
    for (int tzIndex = 0;; tzIndex++)
    {
        char path[64];
        snprintf(path, sizeof(path), "/sys/class/thermal/thermal_zone%d/type", tzIndex);

        FFstrbuf fileContents;
        ffStrbufInit(&fileContents);

        if (!ffGetFileContent(path, &fileContents))
        {
            // this termal zone does not exist
            ffStrbufDestroy(&fileContents);
            break;
        }

        bool isTargetTZ = false;
        for(size_t i = 0; i < sizeof(FF_CPU_THERMAL_ZONES) / sizeof(FF_CPU_THERMAL_ZONES[0]); i++)
        {
            if (strncmp(fileContents.chars, FF_CPU_THERMAL_ZONES[i], fileContents.length) == 0) {
                isTargetTZ = true;
                break;
            }
        }
        if(isTargetTZ)
        {
            snprintf(path, sizeof(path), "/sys/class/thermal/thermal_zone%d/temp", tzIndex);
            ffStrbufInit(&fileContents);
            if (!ffGetFileContent(path, &fileContents))
            {
                ffStrbufDestroy(&fileContents);
                break;
            }
            // The temperature is encoded in millicelsius, meaning "49000" is 49C
            // NOTE: for strtol errno MUST be manually reset (see manpage)
            errno = 0;
            long sensorValue = strtol(fileContents.chars, NULL, 10);
            if (!errno)
                cpuPackageTemp = (float)sensorValue / 1000.0f; // convert to celsius
            ffStrbufDestroy(&fileContents);
            break;
        }
        ffStrbufDestroy(&fileContents);

    }

    if (cpuPackageTemp == __DBL_MAX__)
    {
        ffPrintError(instance, FF_CPU_MODULE_NAME, 0, &instance->config.cpuKey, &instance->config.cpuFormat, FF_CPU_NUM_FORMAT_ARGS, "Could not find a CPU thermal zone");
    }

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
        ffPrintError(instance, FF_CPU_MODULE_NAME, 0, &instance->config.cpuKey, &instance->config.cpuFormat, FF_CPU_NUM_FORMAT_ARGS, "No CPU info found in /proc/cpuinfo");
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
    
    if (cpuPackageTemp != __DBL_MAX__)
        ffStrbufAppendF(&cpu, " (%g°C)", cpuPackageTemp);

    ffPrintAndWriteToCache(instance, FF_CPU_MODULE_NAME, &instance->config.cpuKey, &cpu, &instance->config.cpuFormat, FF_CPU_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRBUF, &name},
        {FF_FORMAT_ARG_TYPE_STRBUF, &namePretty},
        {FF_FORMAT_ARG_TYPE_STRBUF, &vendor},
        {FF_FORMAT_ARG_TYPE_INT, &numProcsOnline},
        {FF_FORMAT_ARG_TYPE_INT, &numProcsAvailable},
        {FF_FORMAT_ARG_TYPE_INT, &physicalCores},
        {FF_FORMAT_ARG_TYPE_INT, &numProcs},
        {FF_FORMAT_ARG_TYPE_DOUBLE, &cpuPackageTemp},
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
