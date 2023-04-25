#include "fastfetch.h"
#include "common/printing.h"
#include "detection/opencl/opencl.h"

#define FF_OPENCL_MODULE_NAME "OpenCL"
#define FF_OPENCL_NUM_FORMAT_ARGS 3

void ffPrintOpenCL(FFinstance* instance)
{
    FFOpenCLResult opencl;
    ffStrbufInit(&opencl.version);
    ffStrbufInit(&opencl.device);
    ffStrbufInit(&opencl.vendor);

    const char* error = ffDetectOpenCL(instance, &opencl);

    if(error != NULL)
        ffPrintError(instance, FF_OPENCL_MODULE_NAME, 0, &instance->config.openCL, "%s", error);
    else
    {
        if(instance->config.openCL.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, FF_OPENCL_MODULE_NAME, 0, &instance->config.openCL.key);
            ffStrbufPutTo(&opencl.version, stdout);
        }
        else
        {
            ffPrintFormat(instance, FF_OPENCL_MODULE_NAME, 0, &instance->config.openCL, FF_OPENCL_NUM_FORMAT_ARGS, (FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_STRBUF, &opencl.version},
                {FF_FORMAT_ARG_TYPE_STRBUF, &opencl.device},
                {FF_FORMAT_ARG_TYPE_STRBUF, &opencl.vendor},
            });
        }
    }

    ffStrbufDestroy(&opencl.version);
    ffStrbufDestroy(&opencl.device);
    ffStrbufDestroy(&opencl.vendor);
}
