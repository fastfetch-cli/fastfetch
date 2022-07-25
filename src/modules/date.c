#include "fastfetch.h"
#include "common/printing.h"

#define FF_DATE_MODULE_NAME "Date"

void ffPrintDate(FFinstance* instance)
{
    if(instance->config.date.outputFormat.length > 0)
    {
        ffPrintDateTimeFormat(instance, FF_DATE_MODULE_NAME, &instance->config.date);
        return;
    }

    const FFDateTimeResult* datetime = ffDetectDateTime(instance);
    ffPrintLogoAndKey(instance, FF_DATE_MODULE_NAME, 0, &instance->config.date.key);

    //yyyy-mm-dd
    printf("%u-%s-%u\n", datetime->year, datetime->monthPretty.chars, datetime->dayInMonth);
}
