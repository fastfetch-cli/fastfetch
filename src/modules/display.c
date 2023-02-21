#include "fastfetch.h"
#include "common/printing.h"
#include "detection/displayserver/displayserver.h"

#define FF_RESOLUTION_MODULE_NAME "Display"
#define FF_RESOLUTION_NUM_FORMAT_ARGS 5

void ffPrintDisplay(FFinstance* instance)
{
    #ifdef __ANDROID__
        ffPrintError(instance, FF_RESOLUTION_MODULE_NAME, 0, &instance->config.display, "Display detection is not supported on Android");
        return;
    #endif

    const FFDisplayServerResult* dsResult = ffConnectDisplayServer(instance);

    for(uint32_t i = 0; i < dsResult->displays.length; i++)
    {
        FFDisplayResult* result = ffListGet(&dsResult->displays, i);
        uint8_t moduleIndex = dsResult->displays.length == 1 ? 0 : (uint8_t) (i + 1);

        if(instance->config.display.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, FF_RESOLUTION_MODULE_NAME, moduleIndex, &instance->config.display.key);
            printf("%ix%i", result->width, result->height);

            if(result->refreshRate > 0)
                printf(" @ %iHz", result->refreshRate);

            if(
                result->scaledWidth > 0 && result->scaledWidth != result->width &&
                result->scaledHeight > 0 && result->scaledHeight != result->height)
                printf(" (as %ix%i)", result->scaledWidth, result->scaledHeight);

            putchar('\n');
        }
        else
        {
            ffPrintFormat(instance, FF_RESOLUTION_MODULE_NAME, moduleIndex, &instance->config.display, FF_RESOLUTION_NUM_FORMAT_ARGS, (FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_UINT, &result->width},
                {FF_FORMAT_ARG_TYPE_UINT, &result->height},
                {FF_FORMAT_ARG_TYPE_UINT, &result->refreshRate},
                {FF_FORMAT_ARG_TYPE_UINT, &result->scaledWidth},
                {FF_FORMAT_ARG_TYPE_UINT, &result->scaledHeight}
            });
        }
    }

    if(dsResult->displays.length == 0)
        ffPrintError(instance, FF_RESOLUTION_MODULE_NAME, 0, &instance->config.display, "Couldn't detect display");
}
