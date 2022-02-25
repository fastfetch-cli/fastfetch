#include "fastfetch.h"

#define FF_TIME_MODULE_NAME "Time"

void ffPrintTime(FFinstance* instance)
{
    if(instance->config.timeFormat.length > 0)
    {
        ffPrintDateTimeFormat(instance, FF_TIME_MODULE_NAME, &instance->config.timeKey, &instance->config.timeFormat);
        return;
    }

    const FFDateTimeResult* datetime = ffDetectDateTime(instance);
    ffPrintLogoAndKey(instance, FF_TIME_MODULE_NAME, 0, &instance->config.timeKey);

    //hh:mm:ss
    printf("%s:%s:%s\n", datetime->hourPretty.chars, datetime->minutePretty.chars, datetime->secondPretty.chars);
}
