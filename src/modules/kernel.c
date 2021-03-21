#include "fastfetch.h"

void ffPrintKernel(FFinstance* instance)
{
    if(ffStrbufIsEmpty(&instance->config.kernelFormat))
    {
        ffPrintLogoAndKey(instance, "Kernel");
        puts(instance->state.utsname.release);
        return;
    }

    FF_STRBUF_CREATE(kernel);

    ffParseFormatString(&kernel, &instance->config.kernelFormat, 3,
        (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.sysname},
        (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.release},
        (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.version}
    );

    ffPrintLogoAndKey(instance, "Kernel");
    ffStrbufWriteTo(&kernel, stdout);
    putchar('\n');
    ffStrbufDestroy(&kernel);
}