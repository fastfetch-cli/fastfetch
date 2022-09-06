#include "displayserver.h"
#include "detection/internal.h"

uint32_t ffdsParseRefreshRate(int32_t refreshRate)
{
    if(refreshRate <= 0)
        return 0;

    int remainder = refreshRate % 5;
    if(remainder >= 3)
        refreshRate += (5 - remainder);
    else
        refreshRate -= remainder;

    //All other typicall refresh rates are dividable by 5
    if(refreshRate == 145)
        refreshRate = 144;

    return (uint32_t) refreshRate;
}

bool ffdsAppendResolution(FFDisplayServerResult* result, uint32_t width, uint32_t height, uint32_t refreshRate)
{
    if(width == 0 || height == 0)
        return false;

    FFResolutionResult* resolution = ffListAdd(&result->resolutions);
    resolution->width = width;
    resolution->height = height;
    resolution->refreshRate = refreshRate;

    return true;
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds, const FFinstance* instance);

const FFDisplayServerResult* ffConnectDisplayServer(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFDisplayServerResult,
        ffConnectDisplayServerImpl(&result, instance);
    );
}
