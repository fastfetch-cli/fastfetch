#include "fastfetch.h"
#include "common/printing.h"
#include "common/networking.h"

#define FF_WEATHER_MODULE_NAME "Weather"
#define FF_WEATHER_NUM_FORMAT_ARGS 1

static int sockfd;

void ffPrepareWeather(FFinstance* instance)
{
    FFstrbuf path;
    ffStrbufInitS(&path, "/?format=");
    ffStrbufAppend(&path, &instance->config.weatherOutputFormat);
    sockfd = ffNetworkingSendHttpRequest("wttr.in", path.chars, "User-Agent: curl/0.0.0\r\n", instance->config.weatherTimeout);
    ffStrbufDestroy(&path);
}

void ffPrintWeather(FFinstance* instance)
{
    if(sockfd == 0)
        ffPrepareWeather(instance);

    if(sockfd < 0)
    {
        ffPrintError(instance, FF_WEATHER_MODULE_NAME, 0, &instance->config.weather, "Failed to connect to 'wttr.in'");
        return;
    }

    FFstrbuf result;
    ffStrbufInitA(&result, 4096);
    ffNetworkingRecvHttpResponse(sockfd, &result);
    ffStrbufSubstrAfterFirstS(&result, "\r\n\r\n");

    if(result.length == 0)
    {
        ffPrintError(instance, FF_WEATHER_MODULE_NAME, 0, &instance->config.weather, "Failed to receive the server response");
        ffStrbufDestroy(&result);
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

    ffStrbufDestroy(&result);
}
