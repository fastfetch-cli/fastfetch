#include "resolution.h"

void ffPrintResolutionValue(FFinstance* instance, uint8_t moduleIndex, FFcache* cache, int width, int height, int refreshRate)
{
    FFstrbuf value;
    ffStrbufInitA(&value, 32);
    ffStrbufAppendF(&value, "%ix%i", width, height);

    if(refreshRate > 0)
        ffStrbufAppendF(&value, " @ %iHz", refreshRate);

    ffPrintAndAppendToCache(instance, FF_RESOLUTION_MODULE_NAME, moduleIndex, &instance->config.resolutionKey, cache, &value, &instance->config.resolutionFormat, FF_RESOLUTION_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_INT, &width},
        {FF_FORMAT_ARG_TYPE_INT, &height},
        {FF_FORMAT_ARG_TYPE_INT, &refreshRate}
    });
}

void ffPrintResolution(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_RESOLUTION_MODULE_NAME, &instance->config.resolutionKey, &instance->config.resolutionFormat, FF_RESOLUTION_NUM_FORMAT_ARGS))
        return;

    if(ffPrintResolutionWaylandBackend(instance))
        return;

    if(ffPrintResolutionXrandrBackend(instance))
        return;

    ffPrintResolutionX11Backend(instance);
}
