#include "weather.h"
#include "common/networking/networking.h"

#define FF_UNITIALIZED ((const char*)(uintptr_t) -1)
static FFNetworkingState state;
static const char* status = FF_UNITIALIZED;

void ffPrepareWeather(FFWeatherOptions* options)
{
    if (status != FF_UNITIALIZED)
    {
        fputs("Error: Weather module can only be used once due to internal limitations\n", stderr);
        exit(1);
    }

    state.timeout = options->timeout;

    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateS("/");
    if (options->location.length)
        ffStrbufAppend(&path, &options->location);
    ffStrbufAppendS(&path, "?format=");
    ffStrbufAppend(&path, &options->outputFormat);
    switch (instance.config.display.tempUnit)
    {
        case FF_TEMPERATURE_UNIT_CELSIUS:
            ffStrbufAppendS(&path, "&m");
            break;
        case FF_TEMPERATURE_UNIT_FAHRENHEIT:
            ffStrbufAppendS(&path, "&u");
            break;
        default:
            break;
    }
    status = ffNetworkingSendHttpRequest(&state, "wttr.in", path.chars, "User-Agent: curl/0.0.0\r\n");
}

const char* ffDetectWeather(FFWeatherOptions* options, FFstrbuf* result)
{
    if(status == FF_UNITIALIZED)
        ffPrepareWeather(options);

    if(status != NULL)
        return status;

    ffStrbufEnsureFree(result, 4095);
    const char* error = ffNetworkingRecvHttpResponse(&state, result);
    if (error == NULL)
    {
        ffStrbufSubstrAfterFirstS(result, "\r\n\r\n");
        ffStrbufTrimRightSpace(result);
    }
    else
        return error;

    if(result->length == 0)
        return "Empty server response received";

    return NULL;
}
