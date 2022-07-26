#include "fastfetch.h"
#include "common/printing.h"
#include "detection/datetime.h"

#define FF_TIME_MODULE_NAME "Time"

void ffPrintTime(FFinstance* instance)
{
    if(instance->config.time.outputFormat.length > 0)
    {
        ffPrintDateTimeFormat(instance, FF_TIME_MODULE_NAME, &instance->config.time);
        return;
    }

    const FFDateTimeResult* datetime = ffDetectDateTime(instance);
    ffPrintLogoAndKey(instance, FF_TIME_MODULE_NAME, 0, &instance->config.time.key);

    //hh:mm:ss
    printf("%s:%s:%s\n", datetime->hourPretty.chars, datetime->minutePretty.chars, datetime->secondPretty.chars);
}
