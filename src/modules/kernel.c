#include "fastfetch.h"
#include "common/printing.h"

#define FF_KERNEL_MODULE_NAME "Kernel"
#define FF_KERNEL_NUM_FORMAT_ARGS 4

void ffPrintKernel(FFinstance* instance)
{
    if(instance->config.kernel.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_KERNEL_MODULE_NAME, 0, &instance->config.kernel.key);
        ffStrbufWriteTo(&instance->state.platform.systemRelease, stdout);

        #ifdef _WIN32
            if(instance->state.platform.systemVersion.length > 0)
                printf(" (%s)", instance->state.platform.systemVersion.chars);
        #endif

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_KERNEL_MODULE_NAME, 0, &instance->config.kernel, FF_KERNEL_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.platform.systemName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.platform.systemRelease},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.platform.systemVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.platform.systemArchitecture}
        });
    }
}
