#include "fastfetch.h"

#define FF_HOST_MODULE_NAME "Host"
#define FF_HOST_NUM_FORMAT_ARGS 3

static inline bool hostValueSet(FFstrbuf* value)
{
    return
        value != NULL &&
        value->length > 0 &&
        ffStrbufIgnCaseCompS(value, "None") != 0 &&
        ffStrbufIgnCaseCompS(value, "To be filled by O.E.M.") != 0
    ;
}

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

    if(!hostValueSet(&family) && !hostValueSet(&name))
    {
        ffPrintError(instance, FF_HOST_MODULE_NAME, 0, &instance->config.hostKey, &instance->config.hostFormat, FF_HOST_NUM_FORMAT_ARGS, "neither family nor name is set by O.E.M.");
        return;
    }

    FF_STRBUF_CREATE(host);

    if(!hostValueSet(&family))
        ffStrbufAppend(&host, &name);
    else if(!hostValueSet(&name))
        ffStrbufAppend(&host, &family);
    else
    {
        ffStrbufAppend(&host, &family);
        ffStrbufAppendC(&host, ' ');
        ffStrbufAppend(&host, &name);
    }

    if(hostValueSet(&version)) {
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
