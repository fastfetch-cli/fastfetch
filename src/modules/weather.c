#include "fastfetch.h"
#include "common/printing.h"
#include "common/networking.h"

#define FF_WEATHER_MODULE_NAME "Weather"
#define FF_WEATHER_NUM_FORMAT_ARGS 1

static FFNetworkingState state;
static int status = -1;

void ffPrepareWeather(FFinstance* instance)
{
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateS("/?format=");
    ffStrbufAppend(&path, &instance->config.weatherOutputFormat);
    status = ffNetworkingSendHttpRequest(&state, "wttr.in", path.chars, "User-Agent: curl/0.0.0\r\n");
}

void ffPrintWeather(FFinstance* instance)
{
    if(status == -1)
        ffPrepareWeather(instance);

    if(status == 0)
    {
        ffPrintError(instance, FF_WEATHER_MODULE_NAME, 0, &instance->config.weather, "Failed to connect to 'wttr.in'");
        return;
    }

    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateA(4096);
    bool success = ffNetworkingRecvHttpResponse(&state, &result, instance->config.weatherTimeout);
    if (success) ffStrbufSubstrAfterFirstS(&result, "\r\n\r\n");

    if(!success || result.length == 0)
    {
        ffPrintError(instance, FF_WEATHER_MODULE_NAME, 0, &instance->config.weather, "Failed to receive the server response");
        return;
    }

    if(instance->config.weather.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_WEATHER_MODULE_NAME, 0, &instance->config.weather.key);
        ffStrbufPutTo(&result, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_WEATHER_MODULE_NAME, 0, &instance->config.weather, FF_WEATHER_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &result}
        });
    }
}
