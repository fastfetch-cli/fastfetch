#include "common/time.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/datetime/datetime.h"
#include "util/stringUtils.h"

#include <time.h>

#pragma GCC diagnostic ignored "-Wformat" // warning: unknown conversion type character 'F' in format

#define FF_DATETIME_DISPLAY_NAME "Date & Time"
#define FF_DATETIME_NUM_FORMAT_ARGS 22

typedef struct FFDateTimeResult
{
    //Examples for 21.02.2022 - 15:18:37
    uint16_t year; //2022
    uint8_t yearShort; //22
    uint8_t month; //2
    char monthPretty[FASTFETCH_STRBUF_DEFAULT_ALLOC]; //02
    char monthName[FASTFETCH_STRBUF_DEFAULT_ALLOC]; //February
    char monthNameShort[FASTFETCH_STRBUF_DEFAULT_ALLOC]; //Feb
    uint8_t week; //8
    char weekday[FASTFETCH_STRBUF_DEFAULT_ALLOC]; //Monday
    char weekdayShort[FASTFETCH_STRBUF_DEFAULT_ALLOC]; //Mon
    uint16_t dayInYear; //52
    uint8_t dayInMonth; //21
    uint8_t dayInWeek; //1
    uint8_t hour; //15
    char hourPretty[FASTFETCH_STRBUF_DEFAULT_ALLOC]; //15
    uint8_t hour12; //3
    char hour12Pretty[FASTFETCH_STRBUF_DEFAULT_ALLOC]; //03
    uint8_t minute; //18
    char minutePretty[FASTFETCH_STRBUF_DEFAULT_ALLOC]; //18
    uint8_t second; //37
    char secondPretty[FASTFETCH_STRBUF_DEFAULT_ALLOC]; //37
    char offsetFromUtc[FASTFETCH_STRBUF_DEFAULT_ALLOC];
    char timezoneName[FASTFETCH_STRBUF_DEFAULT_ALLOC];
} FFDateTimeResult;

void ffPrintDateTimeFormat(struct tm* tm, const FFModuleArgs* moduleArgs)
{
    FFDateTimeResult result;

    result.year = (uint16_t) (tm->tm_year + 1900);
    result.yearShort = (uint8_t) (result.year % 100);
    result.month = (uint8_t) (tm->tm_mon + 1);
    strftime(result.monthPretty, sizeof(result.monthPretty), "%m", tm);
    strftime(result.monthName, sizeof(result.monthName), "%B", tm);
    strftime(result.monthNameShort, sizeof(result.monthNameShort), "%b", tm);
    result.week = (uint8_t) (tm->tm_yday / 7 + 1);
    strftime(result.weekday, sizeof(result.weekday), "%A", tm);
    strftime(result.weekdayShort, sizeof(result.weekdayShort), "%a", tm);
    result.dayInYear = (uint8_t) (tm->tm_yday + 1);
    result.dayInMonth = (uint8_t) tm->tm_mday;
    result.dayInWeek = tm->tm_wday == 0 ? 7 : (uint8_t) tm->tm_wday;
    result.hour = (uint8_t) tm->tm_hour;
    strftime(result.hourPretty, sizeof(result.hourPretty), "%H", tm);
    result.hour12 = (uint8_t) (result.hour % 12);
    strftime(result.hour12Pretty, sizeof(result.hour12Pretty), "%I", tm);
    result.minute = (uint8_t) tm->tm_min;
    strftime(result.minutePretty, sizeof(result.minutePretty), "%M", tm);
    result.second = (uint8_t) tm->tm_sec;
    strftime(result.secondPretty, sizeof(result.secondPretty), "%S", tm);
    strftime(result.offsetFromUtc, sizeof(result.offsetFromUtc), "%z", tm);
    strftime(result.timezoneName, sizeof(result.timezoneName), "%Z", tm);

    FF_PRINT_FORMAT_CHECKED(FF_DATETIME_DISPLAY_NAME, 0, moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_DATETIME_NUM_FORMAT_ARGS, ((FFformatarg[]) {
        {FF_FORMAT_ARG_TYPE_UINT16, &result.year}, // 1
        {FF_FORMAT_ARG_TYPE_UINT8, &result.yearShort}, // 2
        {FF_FORMAT_ARG_TYPE_UINT8, &result.month}, // 3
        {FF_FORMAT_ARG_TYPE_STRING, result.monthPretty}, // 4
        {FF_FORMAT_ARG_TYPE_STRING, result.monthName}, // 5
        {FF_FORMAT_ARG_TYPE_STRING, result.monthNameShort}, // 6
        {FF_FORMAT_ARG_TYPE_UINT8, &result.week}, // 7
        {FF_FORMAT_ARG_TYPE_STRING, result.weekday}, // 8
        {FF_FORMAT_ARG_TYPE_STRING, result.weekdayShort}, // 9
        {FF_FORMAT_ARG_TYPE_UINT16, &result.dayInYear}, // 10
        {FF_FORMAT_ARG_TYPE_UINT8, &result.dayInMonth}, // 11
        {FF_FORMAT_ARG_TYPE_UINT8, &result.dayInWeek}, // 12
        {FF_FORMAT_ARG_TYPE_UINT8, &result.hour}, // 13
        {FF_FORMAT_ARG_TYPE_STRING, result.hourPretty}, // 14
        {FF_FORMAT_ARG_TYPE_UINT8, &result.hour12}, // 15
        {FF_FORMAT_ARG_TYPE_STRING, result.hour12Pretty}, // 16
        {FF_FORMAT_ARG_TYPE_UINT8, &result.minute}, // 17
        {FF_FORMAT_ARG_TYPE_STRING, result.minutePretty}, // 18
        {FF_FORMAT_ARG_TYPE_UINT8, &result.second}, // 19
        {FF_FORMAT_ARG_TYPE_STRING, result.secondPretty}, // 20
        {FF_FORMAT_ARG_TYPE_STRING, result.offsetFromUtc}, // 21
        {FF_FORMAT_ARG_TYPE_STRING, result.timezoneName}, // 22
    }));
}

void ffPrintDateTime(FFDateTimeOptions* options)
{
    uint64_t msNow = ffTimeGetNow();
    time_t sNow = (time_t) (msNow / 1000);
    struct tm* tm = localtime(&sNow);

    if(options->moduleArgs.outputFormat.length > 0)
    {
        ffPrintDateTimeFormat(tm, &options->moduleArgs);
        return;
    }

    char buffer[32];
    if (strftime(buffer, sizeof(buffer), "%F %T", tm) == 0) //yyyy-MM-dd HH:mm:ss
    {
        ffPrintError(FF_DATETIME_DISPLAY_NAME, 0, &options->moduleArgs, "strftime() failed");
        return;
    }

    ffPrintLogoAndKey(FF_DATETIME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

    puts(buffer);
}

bool ffParseDateTimeCommandOptions(FFDateTimeOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_DATETIME_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
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

void ffGenerateDateTimeJsonConfig(FFDateTimeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyDateTimeOptions))) FFDateTimeOptions defaultOptions;
    ffInitDateTimeOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateDateTimeJsonResult(FF_MAYBE_UNUSED FFDateTimeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    yyjson_mut_obj_add_uint(doc, module, "result", ffTimeGetNow());
}

void ffPrintDateTimeHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_DATETIME_MODULE_NAME, "{1}-{4}-{11} {14}:{18}:{20}", FF_DATETIME_NUM_FORMAT_ARGS, ((const char* []) {
        "year",
        "last two digits of year",
        "month",
        "month with leading zero",
        "month name",
        "month name short",
        "week number on year",
        "weekday",
        "weekday short",
        "day in year",
        "day in month",
        "day in Week",
        "hour",
        "hour with leading zero",
        "hour 12h format",
        "hour 12h format with leading zero",
        "minute",
        "minute with leading zero",
        "second",
        "second with leading zero",
        "offset from UTC in the ISO 8601 format",
        "locale-dependent timezone name or abbreviation",
    }));
}

void ffInitDateTimeOptions(FFDateTimeOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_DATETIME_MODULE_NAME,
        "Print current date and time",
        ffParseDateTimeCommandOptions,
        ffParseDateTimeJsonObject,
        ffPrintDateTime,
        ffGenerateDateTimeJsonResult,
        ffPrintDateTimeHelpFormat,
        ffGenerateDateTimeJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyDateTimeOptions(FFDateTimeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
