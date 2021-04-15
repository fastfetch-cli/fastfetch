#include "fastfetch.h"

#define FF_HOST_MODULE_NAME "Host"
#define FF_HOST_NUM_FORMAT_ARGS 3

void ffPrintHost(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_HOST_MODULE_NAME, &instance->config.hostKey, &instance->config.hostFormat, FF_HOST_NUM_FORMAT_ARGS))
        return;

    FF_STRBUF_CREATE(family);
    ffGetFileContent("/sys/devices/virtual/dmi/id/product_family", &family);

    FF_STRBUF_CREATE(name);
    ffGetFileContent("/sys/devices/virtual/dmi/id/product_name", &name);

    FF_STRBUF_CREATE(version);
    ffGetFileContent("/sys/devices/virtual/dmi/id/product_version", &version);

    if(family.length == 0 && name.length == 0)
    {
        ffPrintError(instance, FF_HOST_MODULE_NAME, 0, &instance->config.hostKey, &instance->config.hostFormat, FF_HOST_NUM_FORMAT_ARGS, "neither family nor name could be determined");
        return;
    }

    FF_STRBUF_CREATE(host);

    if(name.length == 0)
    {
        ffStrbufAppend(&host, &family);
    }
    else if(family.length == 0)
    {
        ffStrbufAppend(&host, &name);
    }
    else
    {
        ffStrbufAppend(&host, &family);
        ffStrbufAppendC(&host, ' ');
        ffStrbufAppend(&host, &name);
    }

    if(
        version.length > 0 &&
        ffStrbufIgnCaseCompS(&version, "None") != 0 &&
        ffStrbufIgnCaseCompS(&version, "To be filled by O.E.M.") != 0
    ) {
        ffStrbufAppendC(&host, ' ');
        ffStrbufAppend(&host, &version);
    }

    ffPrintAndSaveToCache(instance, FF_HOST_MODULE_NAME, &instance->config.hostKey, &host, &instance->config.hostFormat, FF_HOST_NUM_FORMAT_ARGS, (FFformatarg[]) {
        {FF_FORMAT_ARG_TYPE_STRBUF, &family},
        {FF_FORMAT_ARG_TYPE_STRBUF, &name},
        {FF_FORMAT_ARG_TYPE_STRBUF, &version}
    });

    ffStrbufDestroy(&family);
    ffStrbufDestroy(&name);
    ffStrbufDestroy(&version);
}
