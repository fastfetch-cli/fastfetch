#include "fastfetch.h"

#define FF_DATE_MODULE_NAME "Date"

void ffPrintDate(FFinstance* instance)
{
    if(instance->config.dateFormat.length > 0)
    {
        ffPrintDateTimeFormat(instance, FF_DATE_MODULE_NAME, &instance->config.dateKey, &instance->config.dateFormat);
        return;
    }

    const FFDateTimeResult* datetime = ffDetectDateTime(instance);
    ffPrintLogoAndKey(instance, FF_DATE_MODULE_NAME, 0, &instance->config.dateKey);

    //yyyy-mm-dd
    printf("%u-%s-%u\n", datetime->year, datetime->monthPretty.chars, datetime->dayInMonth);
}
