#include "weather.h"

static FFNetworkingState state;
static int status = -1;

void ffPrepareWeather(FFWeatherOptions* options)
{
    if (status != -1)
    {
        fputs("Error: this module can only be used once due to internal limitations\n", stderr);
        exit(1);
    }

    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateS("/");
    if (options->location.length)
        ffStrbufAppend(&path, &options->location);
    ffStrbufAppendS(&path, "?format=");
    ffStrbufAppend(&path, &options->outputFormat);
    status = ffNetworkingSendHttpRequest(&state, "wttr.in", path.chars, "User-Agent: curl/0.0.0\r\n");
}

const char* ffDetectWeather(FFWeatherOptions* options, FFstrbuf* result)
{
    if(status == -1)
        ffPrepareWeather(options);

    if(status == 0)
        return "Failed to connect to 'wttr.in'";

    ffStrbufEnsureFree(result, 4095);
    bool success = ffNetworkingRecvHttpResponse(&state, result, options->timeout);
    if (success)
    {
        ffStrbufSubstrAfterFirstS(result, "\r\n\r\n");
        ffStrbufTrimRightSpace(result);
    }

    if(!success || result->length == 0)
        return "Failed to receive the server response";

    return NULL;
}
