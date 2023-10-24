#include "common/time.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/datetime/datetime.h"
#include "util/stringUtils.h"

#include <time.h>

#define FF_DATETIME_DISPLAY_NAME "Date & Time"
#define FF_DATETIME_NUM_FORMAT_ARGS 20

typedef struct FFDateTimeResult
{
    //Examples for 21.02.2022 - 15:18:37
    uint16_t year; //2022
    uint8_t yearShort; //22
    uint8_t month; //2
    FFstrbuf monthPretty; //02
    FFstrbuf monthName; //February
    FFstrbuf monthNameShort; //Feb
    uint8_t week; //8
    FFstrbuf weekday; //Monday
    FFstrbuf weekdayShort; //Mon
    uint16_t dayInYear; //52
    uint8_t dayInMonth; //21
    uint8_t dayInWeek; //1
    uint8_t hour; //15
    FFstrbuf hourPretty; //15
    uint8_t hour12; //3
    FFstrbuf hour12Pretty; //03
    uint8_t minute; //18
    FFstrbuf minutePretty; //18
    uint8_t second; //37
    FFstrbuf secondPretty; //37
} FFDateTimeResult;

void ffPrintDateTimeFormat(struct tm* tm, const FFModuleArgs* moduleArgs)
{
    FFDateTimeResult result;

    result.year = (uint16_t) (tm->tm_year + 1900);
    result.yearShort = (uint8_t) (result.year % 100);
    result.month = (uint8_t) (tm->tm_mon + 1);

    ffStrbufInitA(&result.monthPretty, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.monthPretty.length = (uint32_t) strftime(result.monthPretty.chars, ffStrbufGetFree(&result.monthPretty), "%m", tm);

    ffStrbufInitA(&result.monthName, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.monthName.length = (uint32_t) strftime(result.monthName.chars, ffStrbufGetFree(&result.monthName), "%B", tm);

    ffStrbufInitA(&result.monthNameShort, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.monthNameShort.length = (uint32_t) strftime(result.monthNameShort.chars, ffStrbufGetFree(&result.monthNameShort), "%b", tm);

    result.week = (uint8_t) (tm->tm_yday / 7 + 1);

    ffStrbufInitA(&result.weekday, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.weekday.length = (uint32_t) strftime(result.weekday.chars, ffStrbufGetFree(&result.weekday), "%A", tm);

    ffStrbufInitA(&result.weekdayShort, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.weekdayShort.length = (uint32_t) strftime(result.weekdayShort.chars, ffStrbufGetFree(&result.weekdayShort), "%a", tm);

    result.dayInYear = (uint8_t) (tm->tm_yday + 1);
    result.dayInMonth = (uint8_t) tm->tm_mday;
    result.dayInWeek = tm->tm_wday == 0 ? 7 : (uint8_t) tm->tm_wday;

    result.hour = (uint8_t) tm->tm_hour;

    ffStrbufInitA(&result.hourPretty, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.hourPretty.length = (uint32_t) strftime(result.hourPretty.chars, ffStrbufGetFree(&result.hourPretty), "%H", tm);

    result.hour12 = (uint8_t) (result.hour % 12);

    ffStrbufInitA(&result.hour12Pretty, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.hour12Pretty.length = (uint32_t) strftime(result.hour12Pretty.chars, ffStrbufGetFree(&result.hour12Pretty), "%I", tm);

    result.minute = (uint8_t) tm->tm_min;

    ffStrbufInitA(&result.minutePretty, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.minutePretty.length = (uint32_t) strftime(result.minutePretty.chars, ffStrbufGetFree(&result.minutePretty), "%M", tm);

    result.second = (uint8_t) tm->tm_sec;

    ffStrbufInitA(&result.secondPretty, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.secondPretty.length = (uint32_t) strftime(result.secondPretty.chars, ffStrbufGetFree(&result.secondPretty), "%S", tm);

    ffPrintFormat(FF_DATETIME_DISPLAY_NAME, 0, moduleArgs, FF_DATETIME_NUM_FORMAT_ARGS, (FFformatarg[]) {
        {FF_FORMAT_ARG_TYPE_UINT16, &result.year}, // 1
        {FF_FORMAT_ARG_TYPE_UINT8, &result.yearShort}, // 2
        {FF_FORMAT_ARG_TYPE_UINT8, &result.month}, // 3
        {FF_FORMAT_ARG_TYPE_STRBUF, &result.monthPretty}, // 4
        {FF_FORMAT_ARG_TYPE_STRBUF, &result.monthName}, // 5
        {FF_FORMAT_ARG_TYPE_STRBUF, &result.monthNameShort}, // 6
        {FF_FORMAT_ARG_TYPE_UINT8, &result.week}, // 7
        {FF_FORMAT_ARG_TYPE_STRBUF, &result.weekday}, // 8
        {FF_FORMAT_ARG_TYPE_STRBUF, &result.weekdayShort}, // 9
        {FF_FORMAT_ARG_TYPE_UINT16, &result.dayInYear}, // 10
        {FF_FORMAT_ARG_TYPE_UINT8, &result.dayInMonth}, // 11
        {FF_FORMAT_ARG_TYPE_UINT8, &result.dayInWeek}, // 12
        {FF_FORMAT_ARG_TYPE_UINT8, &result.hour}, // 13
        {FF_FORMAT_ARG_TYPE_STRBUF, &result.hourPretty}, // 14
        {FF_FORMAT_ARG_TYPE_UINT8, &result.hour12}, // 15
        {FF_FORMAT_ARG_TYPE_STRBUF, &result.hour12Pretty}, // 16
        {FF_FORMAT_ARG_TYPE_UINT8, &result.minute}, // 17
        {FF_FORMAT_ARG_TYPE_STRBUF, &result.minutePretty}, // 18
        {FF_FORMAT_ARG_TYPE_UINT8, &result.second}, // 19
        {FF_FORMAT_ARG_TYPE_STRBUF, &result.secondPretty} // 20
    });

    ffStrbufDestroy(&result.hour12Pretty);
    ffStrbufDestroy(&result.hourPretty);
    ffStrbufDestroy(&result.minutePretty);
    ffStrbufDestroy(&result.monthName);
    ffStrbufDestroy(&result.monthNameShort);
    ffStrbufDestroy(&result.monthPretty);
    ffStrbufDestroy(&result.secondPretty);
    ffStrbufDestroy(&result.weekday);
    ffStrbufDestroy(&result.weekdayShort);
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
    ffPrintModuleFormatHelp(FF_DATETIME_MODULE_NAME, "{1}-{4}-{11} {14}:{18}:{20}", FF_DATETIME_NUM_FORMAT_ARGS, (const char* []) {
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
        "second with leading zero"
    });
}

void ffInitDateTimeOptions(FFDateTimeOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_DATETIME_MODULE_NAME,
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
