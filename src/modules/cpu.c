#include "fastfetch.h"

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
    if(ffPrintCachedValue(instance, "CPU"))
        return;

    char name[256];   name[0] = '\0';
    char vendor[256]; vendor[0] = '\0';
    int physicalCores = 0;

    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");

    char* line = NULL;
    size_t len = 0;

    while(getline(&line, &len, cpuinfo) != -1)
    {
        sscanf(line, "model name%*s %[^\n]", name);
        sscanf(line, "vendor_id%*s %[^\n]", vendor);
        sscanf(line, "cpu cores%*s %i", &physicalCores);

        //Stop after the first CPU
        if(strstr(line, "flags") != NULL)
            break;
    }

    if(line != NULL)
        free(line);

    fclose(cpuinfo);

    FFstrbuf namePretty;
    ffStrbufInitA(&namePretty, 256);
    ffStrbufAppendS(&namePretty, name);
    ffStrbufRemoveStrings(&namePretty, 11,
        "(R)", "(r)", "(TM)", "(tm)", " CPU", " FPU", " Processor", " Dual-Core", " Quad-Core", " Six-Core", " Eight-Core"
    );
    ffStrbufLimitLength(&namePretty, ffStrbufLastIndexC(&namePretty, '@')); //Cut the speed output in the name as we append our own
    ffStrbufTrimRight(&namePretty, ' '); //If we removed the @ in previous step there was most likely a space before it

    double biosLimit      = getGhz("/sys/devices/system/cpu/cpu0/cpufreq/bios_limit");
    double scalingMaxFreq = getGhz("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
    double scalingMinFreq = getGhz("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
    double infoMaxFreq    = getGhz("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    double infoMinFreq    = getGhz("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq");

    int numProcsOnline = get_nprocs();
    int numProcsAvailable = get_nprocs_conf();

    FF_STRBUF_CREATE(cpu);

    if(instance->config.cpuFormat.length == 0)
    {
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

        if(namePretty.length > 0)
            ffStrbufAppend(&cpu, &namePretty);
        else if(name[0] != '\0')
            ffStrbufAppendS(&cpu, name);
        else if(vendor[0] != '\0')
        {
            ffStrbufAppendS(&cpu, vendor);
            ffStrbufAppendS(&cpu, " unknown processor");
        }
        else if(numProcs > 1 || ghz > 0)
            ffStrbufAppendS(&cpu, " unknwon processor");
        else
        {
            ffPrintError(instance, "CPU", "No CPU info found in /proc/cpuinfo");
            ffStrbufDestroy(&cpu);
            ffStrbufDestroy(&namePretty);
            return;
        }

        if(numProcs > 1)
            ffStrbufAppendF(&cpu, " (%i)", numProcs);

        if(ghz > 0)
            ffStrbufAppendF(&cpu, " @ %.9gGHz", ghz);
    }
    else
    {
        ffParseFormatString(&cpu, &instance->config.cpuFormat, 11,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, name},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &namePretty},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, vendor},
            (FFformatarg){FF_FORMAT_ARG_TYPE_INT, &numProcsOnline},
            (FFformatarg){FF_FORMAT_ARG_TYPE_INT, &numProcsAvailable},
            (FFformatarg){FF_FORMAT_ARG_TYPE_INT, &physicalCores},
            (FFformatarg){FF_FORMAT_ARG_TYPE_DOUBLE, &biosLimit},
            (FFformatarg){FF_FORMAT_ARG_TYPE_DOUBLE, &scalingMaxFreq},
            (FFformatarg){FF_FORMAT_ARG_TYPE_DOUBLE, &scalingMinFreq},
            (FFformatarg){FF_FORMAT_ARG_TYPE_DOUBLE, &infoMaxFreq},
            (FFformatarg){FF_FORMAT_ARG_TYPE_DOUBLE, &infoMinFreq}
        );
    }

    ffPrintAndSaveCachedValue(instance, "CPU", &cpu);
    ffStrbufDestroy(&cpu);

    ffStrbufDestroy(&namePretty);
}
