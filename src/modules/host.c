#include "fastfetch.h"
#include "common/printing.h"
#include "common/caching.h"
#include "detection/host/host.h"

#define FF_HOST_MODULE_NAME "Host"
#define FF_HOST_NUM_FORMAT_ARGS 15

void ffPrintHost(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_HOST_MODULE_NAME, &instance->config.host, FF_HOST_NUM_FORMAT_ARGS))
        return;

    const FFHostResult* host = ffDetectHost();

    if(host->productFamily.length == 0 && host->productName.length == 0)
    {
        ffPrintError(instance, FF_HOST_MODULE_NAME, 0, &instance->config.host, "neither product_family nor product_name is set by O.E.M.");
        return;
    }

    FFstrbuf output;
    ffStrbufInit(&output);

    if(host->productName.length > 0)
        ffStrbufAppend(&output, &host->productName);
    else
        ffStrbufAppend(&output, &host->productFamily);

    if(host->productVersion.length > 0)
    {
        ffStrbufAppendC(&output, ' ');
        ffStrbufAppend(&output, &host->productVersion);
    }

    ffPrintAndWriteToCache(instance, FF_HOST_MODULE_NAME, &instance->config.host, &output, FF_HOST_NUM_FORMAT_ARGS, (FFformatarg[]) {
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->productFamily},
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->productName},
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->productVersion},
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->productSku},
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->biosDate},
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->biosRelease},
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->biosVendor},
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->biosVersion},
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->boardName},
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->boardVendor},
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->boardVersion},
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->chassisType},
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->chassisVendor},
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->chassisVersion},
        {FF_FORMAT_ARG_TYPE_STRBUF, &host->sysVendor}
    });
}
