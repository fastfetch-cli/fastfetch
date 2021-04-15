#include "fastfetch.h"

#define FF_KERNEL_MODULE_NAME "Kernel"
#define FF_KERNEL_NUM_FORMAT_ARGS 3

void ffPrintKernel(FFinstance* instance)
{
    if(instance->config.kernelFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_KERNEL_MODULE_NAME, 0, &instance->config.kernelKey);
        puts(instance->state.utsname.release);
    }
    else
    {
        ffPrintFormatString(instance, FF_KERNEL_MODULE_NAME, 0, &instance->config.kernelKey, &instance->config.kernelFormat, NULL, FF_KERNEL_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.sysname},
            {FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.release},
            {FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.version}
        });
    }
}
