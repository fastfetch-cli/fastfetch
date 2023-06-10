#include "fastfetch.h"
#include "common/printing.h"
#include "common/networking.h"
#include "modules/weather/weather.h"

#define FF_WEATHER_MODULE_NAME "Weather"
#define FF_WEATHER_NUM_FORMAT_ARGS 1

static FFNetworkingState state;
static int status = -1;

void ffPrepareWeather(FFWeatherOptions* options)
{
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateS("/?format=");
    ffStrbufAppend(&path, &options->outputFormat);
    status = ffNetworkingSendHttpRequest(&state, "wttr.in", path.chars, "User-Agent: curl/0.0.0\r\n");
}

void ffPrintWeather(FFinstance* instance, FFWeatherOptions* options)
{
    if(status == -1)
        ffPrepareWeather(options);

    if(status == 0)
    {
        ffPrintError(instance, FF_WEATHER_MODULE_NAME, 0, &options->moduleArgs, "Failed to connect to 'wttr.in'");
        return;
    }

    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateA(4096);
    bool success = ffNetworkingRecvHttpResponse(&state, &result, options->timeout);
    if (success) ffStrbufSubstrAfterFirstS(&result, "\r\n\r\n");

    if(!success || result.length == 0)
    {
        ffPrintError(instance, FF_WEATHER_MODULE_NAME, 0, &options->moduleArgs, "Failed to receive the server response");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_WEATHER_MODULE_NAME, 0, &options->moduleArgs.key);
        ffStrbufPutTo(&result, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_WEATHER_MODULE_NAME, 0, &options->moduleArgs, FF_WEATHER_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &result}
        });
    }
}

void ffInitWeatherOptions(FFWeatherOptions* options)
{
    options->moduleName = FF_WEATHER_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);

    ffStrbufInitS(&options->outputFormat, "%t+-+%C+(%l)");
    options->timeout = 0;
}

bool ffParseWeatherCommandOptions(FFWeatherOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_WEATHER_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (strcasecmp(subKey, "output-format") == 0)
    {
        ffOptionParseString(key, value, &options->outputFormat);
        return true;
    }

    if (strcasecmp(subKey, "timeout") == 0)
    {
        options->timeout = ffOptionParseUInt32(key, value);
        return true;
    }

    return false;
}

void ffDestroyWeatherOptions(FFWeatherOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);

    ffStrbufDestroy(&options->outputFormat);
}

#ifdef FF_HAVE_JSONC
void ffParseWeatherJsonObject(FFinstance* instance, json_object* module)
{
    FFWeatherOptions __attribute__((__cleanup__(ffDestroyWeatherOptions))) options;
    ffInitWeatherOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            if (strcasecmp(key, "outputFormat") == 0)
            {
                ffStrbufSetS(&options.outputFormat, json_object_get_string(val));
                continue;
            }

            if (strcasecmp(key, "timeout") == 0)
            {
                options.timeout = (uint32_t) json_object_get_int(val);
                continue;
            }

            ffPrintError(instance, FF_WEATHER_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintWeather(instance, &options);
}
#endif
