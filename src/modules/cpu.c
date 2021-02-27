#include "fastfetch.h"

void ffPrintCPU(FFinstance* instance)
{
    if(ffPrintCachedValue(instance, "CPU"))
        return;

    char name[256];
    ffParsePropFile("/proc/cpuinfo", "model name%*s %[^\n]", name);
    if(name[0] == '\0')
    {
        ffPrintError(instance, "CPU", "\"model name%*s %[^\\n]\" not found in \"/proc/cpuinfo\"");
        return;
    }
    
    FILE* frequencyFile = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "r");
    if(frequencyFile == NULL)
    {
        ffPrintError(instance, "CPU", "fopen(\"/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq\", \"r\") == NULL");
        return;
    }
    uint32_t frequency;
    int scanned = fscanf(frequencyFile, "%u", &frequency);
    if(scanned != 1)
    {
        ffPrintError(instance, "CPU", "fscanf(frequencyFile, \"%s\", frequency) != 1");
        return;
    }
    fclose(frequencyFile);

    frequency /= 1000;                        //to MHz
    double ghz = (double) frequency / 1000.0; //to GHz

    char value[1024];
    sprintf(value, "%s (%i) @ %.9gGHz", name, get_nprocs(), ghz);
    
    ffPrintAndSaveCachedValue(instance, "CPU", value);
}