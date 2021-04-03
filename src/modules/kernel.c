#include "fastfetch.h"

void ffPrintKernel(FFinstance* instance)
{
    if(instance->config.kernelFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, &instance->config.kernelKey, "Kernel");
        puts(instance->state.utsname.release);
    }
    else
    {
        ffPrintFormatString(instance, &instance->config.kernelKey, "Kernel", &instance->config.kernelFormat, 3,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.sysname},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.release},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.version}
        );
    }
}
