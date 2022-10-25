#include "fastfetch.h"
#include "common/printing.h"

#define FF_KERNEL_MODULE_NAME "Kernel"
#define FF_KERNEL_NUM_FORMAT_ARGS 3

void ffPrintKernel(FFinstance* instance)
{
    if(instance->config.kernel.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_KERNEL_MODULE_NAME, 0, &instance->config.kernel.key);

        #ifdef _WIN32
            if(instance->state.utsname.version[0] != '\0')
                printf("%s (%s)\n", instance->state.utsname.release, instance->state.utsname.version);
            else
                puts(instance->state.utsname.release);
        #else
            puts(instance->state.utsname.release);
        #endif
    }
    else
    {
        ffPrintFormat(instance, FF_KERNEL_MODULE_NAME, 0, &instance->config.kernel, FF_KERNEL_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.sysname},
            {FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.release},
            {FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.version}
        });
    }
}
