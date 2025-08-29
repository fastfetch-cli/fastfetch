#include "common/time.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/datetime/datetime.h"
#include "util/stringUtils.h"

#include <time.h>

#pragma GCC diagnostic ignored "-Wformat" // warning: unknown conversion type character 'F' in format

#define FF_DATETIME_DISPLAY_NAME "Date & Time"

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
    char dayPretty[FASTFETCH_STRBUF_DEFAULT_ALLOC]; //01
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

static void printDateTimeFormat(struct tm* tm, const FFModuleArgs* moduleArgs)
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
    strftime(result.dayPretty, sizeof(result.dayPretty), "%d", tm);
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

    FF_PRINT_FORMAT_CHECKED(FF_DATETIME_DISPLAY_NAME, 0, moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
        FF_FORMAT_ARG(result.year, "year"), // 1
        FF_FORMAT_ARG(result.yearShort, "year-short"), // 2
        FF_FORMAT_ARG(result.month, "month"), // 3
        FF_FORMAT_ARG(result.monthPretty, "month-pretty"), // 4
        FF_FORMAT_ARG(result.monthName, "month-name"), // 5
        FF_FORMAT_ARG(result.monthNameShort, "month-name-short"), // 6
        FF_FORMAT_ARG(result.week, "week"), // 7
        FF_FORMAT_ARG(result.weekday, "weekday"), // 8
        FF_FORMAT_ARG(result.weekdayShort, "weekday-short"), // 9
        FF_FORMAT_ARG(result.dayInYear, "day-in-year"), // 10
        FF_FORMAT_ARG(result.dayInMonth, "day-in-month"), // 11
        FF_FORMAT_ARG(result.dayInWeek, "day-in-week"), // 12
        FF_FORMAT_ARG(result.hour, "hour"), // 13
        FF_FORMAT_ARG(result.hourPretty, "hour-pretty"), // 14
        FF_FORMAT_ARG(result.hour12, "hour-12"), // 15
        FF_FORMAT_ARG(result.hour12Pretty, "hour-12-pretty"), // 16
        FF_FORMAT_ARG(result.minute, "minute"), // 17
        FF_FORMAT_ARG(result.minutePretty, "minute-pretty"), // 18
        FF_FORMAT_ARG(result.second, "second"), // 19
        FF_FORMAT_ARG(result.secondPretty, "second-pretty"), // 20
        FF_FORMAT_ARG(result.offsetFromUtc, "offset-from-utc"), // 21
        FF_FORMAT_ARG(result.timezoneName, "timezone-name"), // 22
        FF_FORMAT_ARG(result.dayPretty, "day-pretty"), // 23
    }));
}

bool ffPrintDateTime(FFDateTimeOptions* options)
{
    uint64_t msNow = ffTimeGetNow();
    time_t sNow = (time_t) (msNow / 1000);
    struct tm* tm = localtime(&sNow);

    if(options->moduleArgs.outputFormat.length > 0)
    {
        printDateTimeFormat(tm, &options->moduleArgs);
        return true;
    }

    char buffer[32];
    if (strftime(buffer, ARRAY_SIZE(buffer), "%F %T", tm) == 0) //yyyy-MM-dd HH:mm:ss
    {
        ffPrintError(FF_DATETIME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "strftime() failed");
        return false;
    }

    ffPrintLogoAndKey(FF_DATETIME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

    puts(buffer);
    return true;
}

void ffParseDateTimeJsonObject(FFDateTimeOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_DATETIME_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateDateTimeJsonConfig(FFDateTimeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateDateTimeJsonResult(FF_MAYBE_UNUSED FFDateTimeOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    yyjson_mut_obj_add_strcpy(doc, module, "result", ffTimeToFullStr(ffTimeGetNow()));
    return true;
}

void ffInitDateTimeOptions(FFDateTimeOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyDateTimeOptions(FFDateTimeOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffDateTimeModuleInfo = {
    .name = FF_DATETIME_MODULE_NAME,
    .description = "Print current date and time",
    .initOptions = (void*) ffInitDateTimeOptions,
    .destroyOptions = (void*) ffDestroyDateTimeOptions,
    .parseJsonObject = (void*) ffParseDateTimeJsonObject,
    .printModule = (void*) ffPrintDateTime,
    .generateJsonResult = (void*) ffGenerateDateTimeJsonResult,
    .generateJsonConfig = (void*) ffGenerateDateTimeJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Year", "year"},
        {"Last two digits of year", "year-short"},
        {"Month", "month"},
        {"Month with leading zero", "month-pretty"},
        {"Month name", "month-name"},
        {"Month name short", "month-name-short"},
        {"Week number on year", "week"},
        {"Weekday", "weekday"},
        {"Weekday short", "weekday-short"},
        {"Day in year", "day-in-year"},
        {"Day in month", "day-in-month"},
        {"Day in week", "day-in-week"},
        {"Hour", "hour"},
        {"Hour with leading zero", "hour-pretty"},
        {"Hour 12h format", "hour-12"},
        {"Hour 12h format with leading zero", "hour-12-pretty"},
        {"Minute", "minute"},
        {"Minute with leading zero", "minute-pretty"},
        {"Second", "second"},
        {"Second with leading zero", "second-pretty"},
        {"Offset from UTC in the ISO 8601 format", "offset-from-utc"},
        {"Locale-dependent timezone name or abbreviation", "timezone-name"},
        {"Day in month with leading zero", "day-pretty"},
    }))
};
