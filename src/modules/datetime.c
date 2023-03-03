#include "fastfetch.h"
#include "common/printing.h"
#include "detection/datetime/datetime.h"

#define FF_DATETIME_MODULE_NAME "Date & Time"
#define FF_DATETIME_NUM_FORMAT_ARGS 20

void ffPrintDateTimeFormat(FFinstance* instance, const char* moduleName, const FFModuleArgs* moduleArgs)
{
    const FFDateTimeResult* result = ffDetectDateTime(instance);
    ffPrintFormat(instance, moduleName, 0, moduleArgs, FF_DATETIME_NUM_FORMAT_ARGS, (FFformatarg[]) {
        {FF_FORMAT_ARG_TYPE_UINT16, &result->year},
        {FF_FORMAT_ARG_TYPE_UINT8, &result->yearShort},
        {FF_FORMAT_ARG_TYPE_UINT8, &result->month},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->monthPretty},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->monthName},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->monthNameShort},
        {FF_FORMAT_ARG_TYPE_UINT8, &result->week},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->weekday},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->weekdayShort},
        {FF_FORMAT_ARG_TYPE_UINT16, &result->dayInYear},
        {FF_FORMAT_ARG_TYPE_UINT8, &result->dayInMonth},
        {FF_FORMAT_ARG_TYPE_UINT8, &result->dayInWeek},
        {FF_FORMAT_ARG_TYPE_UINT8, &result->hour},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->hourPretty},
        {FF_FORMAT_ARG_TYPE_UINT8, &result->hour12},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->hour12Pretty},
        {FF_FORMAT_ARG_TYPE_UINT8, &result->minute},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->minutePretty},
        {FF_FORMAT_ARG_TYPE_UINT8, &result->second},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->secondPretty}
    });
}

void ffPrintDateTime(FFinstance* instance)
{
    if(instance->config.dateTime.outputFormat.length > 0)
    {
        ffPrintDateTimeFormat(instance, FF_DATETIME_MODULE_NAME, &instance->config.dateTime);
        return;
    }

    const FFDateTimeResult* datetime = ffDetectDateTime(instance);
    ffPrintLogoAndKey(instance, FF_DATETIME_MODULE_NAME, 0, &instance->config.dateTime.key);

    //yyyy-MM-dd HH:mm:ss
    printf("%u-%s-%02u %s:%s:%s\n", datetime->year, datetime->monthPretty.chars, datetime->dayInMonth, datetime->hourPretty.chars, datetime->minutePretty.chars, datetime->secondPretty.chars);
}
