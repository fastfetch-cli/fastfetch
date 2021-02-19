#include "fastfetch.h"

#include <float.h>

void ffPrintCPU(FFstate* state)
{
    char name[256];
    ffParsePropFile("/proc/cpuinfo", "model name%*s %[^\n]", name);
    
    char frequency[256];
    ffReadFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", frequency, 256);

    char cores[256];

    uint32_t hz;
    sscanf(frequency, "%u", &hz);      //in KHz
    hz /= 1000;                        //to MHz
    double ghz = (double) hz / 1000.0; //to GHz

    ffPrintLogoAndKey(state, "CPU");
    printf("%s (%i) @ %.9gGHz\n", name, get_nprocs(), ghz);
}