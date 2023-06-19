#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/uptime/uptime.h"
#include "modules/uptime/uptime.h"
#include "util/stringUtils.h"

#define FF_UPTIME_NUM_FORMAT_ARGS 4

void ffPrintUptime(FFinstance* instance, FFUptimeOptions* options)
{
    uint64_t uptime;

    const char* error = ffDetectUptime(&uptime);

    if(error)
    {
        ffPrintError(instance, FF_UPTIME_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    uint32_t days    = (uint32_t)  uptime / 86400;
    uint32_t hours   = (uint32_t) (uptime - (days * 86400)) / 3600;
    uint32_t minutes = (uint32_t) (uptime - (days * 86400) - (hours * 3600)) / 60;
    uint32_t seconds = (uint32_t)  uptime - (days * 86400) - (hours * 3600) - (minutes * 60);

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_UPTIME_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);

        if(days == 0 && hours == 0 && minutes == 0)
        {
            printf("%u seconds\n", seconds);
            return;
        }

        if(days > 0)
        {
            printf("%u day", days);

            if(days > 1)
                putchar('s');

            if(days >= 100)
                fputs("(!)", stdout);

            if(hours > 0 || minutes > 0)
                fputs(", ", stdout);
        }

        if(hours > 0)
        {
            printf("%u hour", hours);

            if(hours > 1)
                putchar('s');

            if(minutes > 0)
                fputs(", ", stdout);
        }

        if(minutes > 0)
        {
            printf("%u min", minutes);

            if(minutes > 1)
                putchar('s');
        }

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_UPTIME_MODULE_NAME, 0, &options->moduleArgs, FF_UPTIME_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT, &days},
            {FF_FORMAT_ARG_TYPE_UINT, &hours},
            {FF_FORMAT_ARG_TYPE_UINT, &minutes},
            {FF_FORMAT_ARG_TYPE_UINT, &seconds}
        });
    }
}

void ffInitUptimeOptions(FFUptimeOptions* options)
{
    options->moduleName = FF_UPTIME_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseUptimeCommandOptions(FFUptimeOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_UPTIME_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyUptimeOptions(FFUptimeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseUptimeJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFUptimeOptions __attribute__((__cleanup__(ffDestroyUptimeOptions))) options;
    ffInitUptimeOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(ffStrEqualsIgnCase(key, "type"))
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_UPTIME_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintUptime(instance, &options);
}
