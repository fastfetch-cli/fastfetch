#include "fastfetch.h"
#include "common/printing.h"
#include "detection/kernel/kernel.h"

#define FF_KERNEL_MODULE_NAME "Kernel"
#define FF_KERNEL_NUM_FORMAT_ARGS 3

void ffPrintKernel(FFinstance* instance)
{
    FFKernelResult result;
    ffDetectKernel(instance, &result);

    if(result.error.length > 0)
    {
        ffPrintError(instance, FF_KERNEL_MODULE_NAME, 0, &instance->config.kernel, "%*s", result.error.length, result.error.chars);
        goto exit;
    }

    if(instance->config.kernel.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_KERNEL_MODULE_NAME, 0, &instance->config.kernel.key);
        ffStrbufPutTo(&result.release, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_KERNEL_MODULE_NAME, 0, &instance->config.kernel, FF_KERNEL_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.sysname},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.release},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.version}
        });
    }

exit:
    ffStrbufDestroy(&result.error);
    ffStrbufDestroy(&result.sysname);
    ffStrbufDestroy(&result.release);
    ffStrbufDestroy(&result.version);
}
