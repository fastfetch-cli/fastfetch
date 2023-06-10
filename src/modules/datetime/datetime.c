#include "fastfetch.h"
#include "common/printing.h"
#include "detection/datetime/datetime.h"
#include "modules/datetime/datetime.h"

#define FF_DATETIME_DISPLAY_NAME "Date & Time"
#define FF_DATETIME_NUM_FORMAT_ARGS 20

void ffPrintDateTimeFormat(FFinstance* instance, const char* moduleName, const FFModuleArgs* moduleArgs)
{
    const FFDateTimeResult* result = ffDetectDateTime(instance);
    ffPrintFormat(instance, moduleName, 0, moduleArgs, FF_DATETIME_NUM_FORMAT_ARGS, (FFformatarg[]) {
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

void ffPrintDateTime(FFinstance* instance, FFDateTimeOptions* options)
{
    if(options->moduleArgs.outputFormat.length > 0)
    {
        ffPrintDateTimeFormat(instance, FF_DATETIME_DISPLAY_NAME, &options->moduleArgs);
        return;
    }

    const FFDateTimeResult* datetime = ffDetectDateTime(instance);
    ffPrintLogoAndKey(instance, FF_DATETIME_DISPLAY_NAME, 0, &options->moduleArgs.key);

    //yyyy-MM-dd HH:mm:ss
    printf("%u-%s-%02u %s:%s:%s\n", datetime->year, datetime->monthPretty.chars, datetime->dayInMonth, datetime->hourPretty.chars, datetime->minutePretty.chars, datetime->secondPretty.chars);
}

void ffInitDateTimeOptions(FFDateTimeOptions* options)
{
    options->moduleName = FF_DATETIME_MODULE_NAME;
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

#ifdef FF_HAVE_JSONC
void ffParseDateTimeJsonObject(FFinstance* instance, json_object* module)
{
    FFDateTimeOptions __attribute__((__cleanup__(ffDestroyDateTimeOptions))) options;
    ffInitDateTimeOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_DATETIME_DISPLAY_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintDateTime(instance, &options);
}
#endif
