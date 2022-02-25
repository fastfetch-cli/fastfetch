#include "fastfetch.h"

#define FF_DATETIME_MODULE_NAME "Date Time"
#define FF_DATETIME_NUM_FORMAT_ARGS 20

void ffPrintDateTimeFormat(FFinstance* instance, const char* moduleName, const FFstrbuf* key, const FFstrbuf* format)
{
    const FFDateTimeResult* result = ffDetectDateTime(instance);
    ffPrintFormatString(instance, moduleName, 0, key, format, NULL, FF_DATETIME_NUM_FORMAT_ARGS, (FFformatarg[]) {
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
    if(instance->config.dateTimeFormat.length > 0)
    {
        ffPrintDateTimeFormat(instance, FF_DATETIME_MODULE_NAME, &instance->config.dateTimeKey, &instance->config.dateTimeFormat);
        return;
    }

    const FFDateTimeResult* datetime = ffDetectDateTime(instance);
    ffPrintLogoAndKey(instance, FF_DATETIME_MODULE_NAME, 0, &instance->config.dateTimeKey);

    //yyyy-mm-dd hh:mm:ss
    printf("%u-%s-%u %s:%s:%s\n", datetime->year, datetime->monthPretty.chars, datetime->dayInMonth, datetime->hourPretty.chars, datetime->minutePretty.chars, datetime->secondPretty.chars);
}
