#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/datetime/datetime.h"
#include "modules/datetime/datetime.h"
#include "util/stringUtils.h"

#define FF_DATETIME_DISPLAY_NAME "Date & Time"
#define FF_DATETIME_NUM_FORMAT_ARGS 20

void ffPrintDateTimeFormat(const char* moduleName, const FFModuleArgs* moduleArgs)
{
    const FFDateTimeResult* result = ffDetectDateTime();
    ffPrintFormat(moduleName, 0, moduleArgs, FF_DATETIME_NUM_FORMAT_ARGS, (FFformatarg[]) {
        {FF_FORMAT_ARG_TYPE_UINT16, &result->year}, // 1
        {FF_FORMAT_ARG_TYPE_UINT8, &result->yearShort}, // 2
        {FF_FORMAT_ARG_TYPE_UINT8, &result->month}, // 3
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->monthPretty}, // 4
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->monthName}, // 5
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->monthNameShort}, // 6
        {FF_FORMAT_ARG_TYPE_UINT8, &result->week}, // 7
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->weekday}, // 8
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->weekdayShort}, // 9
        {FF_FORMAT_ARG_TYPE_UINT16, &result->dayInYear}, // 10
        {FF_FORMAT_ARG_TYPE_UINT8, &result->dayInMonth}, // 11
        {FF_FORMAT_ARG_TYPE_UINT8, &result->dayInWeek}, // 12
        {FF_FORMAT_ARG_TYPE_UINT8, &result->hour}, // 13
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->hourPretty}, // 14
        {FF_FORMAT_ARG_TYPE_UINT8, &result->hour12}, // 15
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->hour12Pretty}, // 16
        {FF_FORMAT_ARG_TYPE_UINT8, &result->minute}, // 17
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->minutePretty}, // 18
        {FF_FORMAT_ARG_TYPE_UINT8, &result->second}, // 19
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->secondPretty} // 20
    });
}

void ffPrintDateTime(FFDateTimeOptions* options)
{
    if(options->moduleArgs.outputFormat.length > 0)
    {
        ffPrintDateTimeFormat(FF_DATETIME_DISPLAY_NAME, &options->moduleArgs);
        return;
    }

    const FFDateTimeResult* datetime = ffDetectDateTime();
    ffPrintLogoAndKey(FF_DATETIME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

    //yyyy-MM-dd HH:mm:ss
    printf("%u-%s-%02u %s:%s:%s\n", datetime->year, datetime->monthPretty.chars, datetime->dayInMonth, datetime->hourPretty.chars, datetime->minutePretty.chars, datetime->secondPretty.chars);
}

void ffInitDateTimeOptions(FFDateTimeOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_DATETIME_MODULE_NAME, ffParseDateTimeCommandOptions, ffParseDateTimeJsonObject, ffPrintDateTime);
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseDateTimeCommandOptions(FFDateTimeOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_DATETIME_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyDateTimeOptions(FFDateTimeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseDateTimeJsonObject(FFDateTimeOptions* options, yyjson_val* module)
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

        ffPrintError(FF_DATETIME_DISPLAY_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}
