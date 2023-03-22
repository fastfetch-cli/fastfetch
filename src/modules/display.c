#include "fastfetch.h"
#include "common/printing.h"
#include "detection/displayserver/displayserver.h"

#define FF_DISPLAY_MODULE_NAME "Display"
#define FF_DISPLAY_NUM_FORMAT_ARGS 6

void ffPrintDisplay(FFinstance* instance)
{
    #ifdef __ANDROID__
        ffPrintError(instance, FF_DISPLAY_MODULE_NAME, 0, &instance->config.display, "Display detection is not supported on Android");
        return;
    #endif

    const FFDisplayServerResult* dsResult = ffConnectDisplayServer(instance);

    if (instance->config.displayCompactType != FF_DISPLAY_COMPACT_TYPE_NONE)
    {
        ffPrintLogoAndKey(instance, FF_DISPLAY_MODULE_NAME, 0, &instance->config.display.key);

        int index = 0;
        FF_LIST_FOR_EACH(FFDisplayResult, result, dsResult->displays)
        {
            if (instance->config.displayCompactType & FF_DISPLAY_COMPACT_TYPE_ORIGINAL_BIT)
            {
                if (index++) putchar(' ');
                printf("%ix%i", result->width, result->height);
            }
            if (instance->config.displayCompactType & FF_DISPLAY_COMPACT_TYPE_SCALED_BIT)
            {
                if (index++) putchar(' ');
                printf("%ix%i", result->scaledWidth, result->scaledHeight);
            }
            ffStrbufDestroy(&result->name);
        }
        putchar('\n');
        return;
    }

    FF_STRBUF_AUTO_DESTROY key;
    ffStrbufInit(&key);

    for(uint32_t i = 0; i < dsResult->displays.length; i++)
    {
        FFDisplayResult* result = ffListGet(&dsResult->displays, i);
        uint8_t moduleIndex = dsResult->displays.length == 1 ? 0 : (uint8_t) (i + 1);

        if(instance->config.display.outputFormat.length == 0)
        {
            if(result->name.length)
            {
                ffStrbufClear(&key);
                if(instance->config.display.key.length == 0)
                {
                    ffStrbufAppendF(&key, "%s (%s)", FF_DISPLAY_MODULE_NAME, result->name.chars);
                }
                else
                {
                    ffParseFormatString(&key, &instance->config.display.key, 1, (FFformatarg[]){
                        {FF_FORMAT_ARG_TYPE_STRBUF, &result->name}
                    });
                }
                ffPrintLogoAndKey(instance, key.chars, 0, NULL);
            }
            else
            {
                ffPrintLogoAndKey(instance, FF_DISPLAY_MODULE_NAME, moduleIndex, &instance->config.display.key);
            }

            printf("%ix%i", result->width, result->height);

            if(result->refreshRate > 0)
                printf(" @ %iHz", (uint32_t) (result->refreshRate + 0.5));

            if(
                result->scaledWidth > 0 && result->scaledWidth != result->width &&
                result->scaledHeight > 0 && result->scaledHeight != result->height)
                printf(" (as %ix%i)", result->scaledWidth, result->scaledHeight);

            putchar('\n');
        }
        else
        {
            ffPrintFormat(instance, FF_DISPLAY_MODULE_NAME, moduleIndex, &instance->config.display, FF_DISPLAY_NUM_FORMAT_ARGS, (FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_UINT, &result->width},
                {FF_FORMAT_ARG_TYPE_UINT, &result->height},
                {FF_FORMAT_ARG_TYPE_DOUBLE, &result->refreshRate},
                {FF_FORMAT_ARG_TYPE_UINT, &result->scaledWidth},
                {FF_FORMAT_ARG_TYPE_UINT, &result->scaledHeight},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result->name},
            });
        }

        ffStrbufDestroy(&result->name);
    }

    if(dsResult->displays.length == 0)
        ffPrintError(instance, FF_DISPLAY_MODULE_NAME, 0, &instance->config.display, "Couldn't detect display");
}
