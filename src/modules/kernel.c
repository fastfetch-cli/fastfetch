#include "fastfetch.h"

void ffPrintKernel(FFinstance* instance)
{
    ffPrintLogoAndKey(instance, "Kernel");

    if(instance->config.kernelShowRelease && instance->config.kernelShowVersion)
        printf("%s %s\n", instance->state.utsname.release, instance->state.utsname.version);
    else if(instance->config.kernelShowRelease)
        puts(instance->state.utsname.release);
    else if(instance->config.kernelShowVersion)
        puts(instance->state.utsname.version);
    else
        puts(instance->state.utsname.sysname);
}