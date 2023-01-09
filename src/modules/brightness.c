#include "fastfetch.h"
#include "common/printing.h"
#include "detection/brightness/brightness.h"

#define FF_BRIGHTNESS_MODULE_NAME "Brightness"
#define FF_BRIGHTNESS_NUM_FORMAT_ARGS 2

void ffPrintBrightness(FFinstance* instance)
{
    FFlist result;
    ffListInit(&result, sizeof(FFBrightnessResult));
    const char* error = ffDetectBrightness(&result);

    if(error)
    {
        ffPrintError(instance, FF_BRIGHTNESS_MODULE_NAME, 0, &instance->config.brightness, "%s", error);
        goto exit;
    }

    if(result.length == 0)
    {
        ffPrintError(instance, FF_BRIGHTNESS_MODULE_NAME, 0, &instance->config.brightness, "No result is detected.");
        goto exit;
    }

    FFstrbuf key;
    ffStrbufInit(&key);

    FF_LIST_FOR_EACH(FFBrightnessResult, item, result)
    {
        if(instance->config.brightness.key.length == 0)
        {
            ffStrbufAppendF(&key, "%s (%s)", FF_BRIGHTNESS_MODULE_NAME, item->name.chars);
        }
        else
        {
            ffParseFormatString(&key, &instance->config.brightness.key, 1, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &item->name}
            });
        }

        if(instance->config.brightness.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, key.chars, 0, NULL);
            printf("%.0f%%\n", item->value);
        }
        else
        {
            ffPrintFormatString(instance, key.chars, 0, NULL, &instance->config.brightness.outputFormat, FF_BRIGHTNESS_NUM_FORMAT_ARGS, (FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_STRBUF, &item->value},
                {FF_FORMAT_ARG_TYPE_FLOAT, &item->name}
            });
        }

        ffStrbufDestroy(&item->name);
    }

    ffStrbufDestroy(&key);

exit:
    ffListDestroy(&result);
}
