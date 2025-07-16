#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/time.h"
#include "detection/uptime/uptime.h"
#include "modules/uptime/uptime.h"
#include "util/stringUtils.h"

void ffPrintUptime(FFUptimeOptions* options)
{
    FFUptimeResult result = {};

    const char* error = ffDetectUptime(&result);

    if(error)
    {
        ffPrintError(FF_UPTIME_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    uint64_t uptime = result.uptime;

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_UPTIME_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
        ffParseDuration((uptime + 500) / 1000, &buffer);

        ffStrbufPutTo(&buffer, stdout);
    }
    else
    {
        uint32_t milliseconds = (uint32_t) (uptime % 1000);
        uptime /= 1000;
        uint32_t seconds = (uint32_t) (uptime % 60);
        uptime /= 60;
        uint32_t minutes = (uint32_t) (uptime % 60);
        uptime /= 60;
        uint32_t hours = (uint32_t) (uptime % 24);
        uptime /= 24;
        uint32_t days = (uint32_t) uptime;

        FFTimeGetAgeResult age = ffTimeGetAge(result.bootTime, ffTimeGetNow());

        FF_PRINT_FORMAT_CHECKED(FF_UPTIME_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(days, "days"),
            FF_FORMAT_ARG(hours, "hours"),
            FF_FORMAT_ARG(minutes, "minutes"),
            FF_FORMAT_ARG(seconds, "seconds"),
            FF_FORMAT_ARG(milliseconds, "milliseconds"),
            {FF_FORMAT_ARG_TYPE_STRING, ffTimeToShortStr(result.bootTime), "boot-time"},
            FF_FORMAT_ARG(age.years, "years"),
            FF_FORMAT_ARG(age.daysOfYear, "days-of-year"),
            FF_FORMAT_ARG(age.yearsFraction, "years-fraction"),
        }));
    }
}

bool ffParseUptimeCommandOptions(FFUptimeOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_UPTIME_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseUptimeJsonObject(FFUptimeOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_UPTIME_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateUptimeJsonConfig(FFUptimeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyUptimeOptions))) FFUptimeOptions defaultOptions;
    ffInitUptimeOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateUptimeJsonResult(FF_MAYBE_UNUSED FFUptimeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFUptimeResult result;
    const char* error = ffDetectUptime(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_uint(doc, obj, "uptime", result.uptime);
    yyjson_mut_obj_add_strcpy(doc, obj, "bootTime", ffTimeToFullStr(result.bootTime));
}

static FFModuleBaseInfo ffModuleInfo = {
    .name = FF_UPTIME_MODULE_NAME,
    .description = "Print how long system has been running",
    .parseCommandOptions = (void*) ffParseUptimeCommandOptions,
    .parseJsonObject = (void*) ffParseUptimeJsonObject,
    .printModule = (void*) ffPrintUptime,
    .generateJsonResult = (void*) ffGenerateUptimeJsonResult,
    .generateJsonConfig = (void*) ffGenerateUptimeJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Days after boot", "days"},
        {"Hours after boot", "hours"},
        {"Minutes after boot", "minutes"},
        {"Seconds after boot", "seconds"},
        {"Milliseconds after boot", "milliseconds"},
        {"Boot time in local timezone", "boot-time"},
        {"Years integer after boot", "years"},
        {"Days of year after boot", "days-of-year"},
        {"Years fraction after boot", "years-fraction"},
    }))
};

void ffInitUptimeOptions(FFUptimeOptions* options)
{
    options->moduleInfo = ffModuleInfo;
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyUptimeOptions(FFUptimeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
