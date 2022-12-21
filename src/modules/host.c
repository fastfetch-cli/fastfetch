#include "fastfetch.h"
#include "common/printing.h"
#include "detection/host/host.h"

#define FF_HOST_MODULE_NAME "Host"
#define FF_HOST_NUM_FORMAT_ARGS 8

void ffPrintHost(FFinstance* instance)
{
    const FFHostResult* host = ffDetectHost();

    if(host->error.length > 0)
    {
        ffPrintError(instance, FF_HOST_MODULE_NAME, 0, &instance->config.host, "%*s", host->error.length, host->error.chars);
        return;
    }

    if(host->productFamily.length == 0 && host->productName.length == 0)
    {
        ffPrintError(instance, FF_HOST_MODULE_NAME, 0, &instance->config.host, "neither product_family nor product_name is set by O.E.M.");
        return;
    }

    if(instance->config.host.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_HOST_MODULE_NAME, 0, &instance->config.host.key);

        FFstrbuf output;
        ffStrbufInit(&output);

        if(host->productName.length > 0)
            ffStrbufAppend(&output, &host->productName);
        else
            ffStrbufAppend(&output, &host->productFamily);

        if(host->productVersion.length > 0)
        {
            ffStrbufAppendF(&output, " (%s)", host->productVersion.chars);
        }

        ffStrbufPutTo(&output, stdout);

        ffStrbufDestroy(&output);
    }
    else
    {
        ffPrintFormat(instance, FF_HOST_MODULE_NAME, 0, &instance->config.host, FF_HOST_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &host->productFamily},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host->productName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host->productVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host->productSku},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host->chassisType},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host->chassisVendor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host->chassisVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host->sysVendor}
        });
    }
}
