#include "fastfetch.h"
#include "common/printing.h"
#include "detection/displayserver/displayserver.h"

#define FF_RESOLUTION_MODULE_NAME "Resolution"
#define FF_RESOLUTION_NUM_FORMAT_ARGS 3

void ffPrintResolution(FFinstance* instance)
{
    #ifdef __ANDROID__
        ffPrintError(instance, FF_RESOLUTION_MODULE_NAME, 0, &instance->config.resolution, "Resolution detection is not supported on Android");
        return;
    #endif

    const FFDisplayServerResult* dsResult = ffConnectDisplayServer(instance);

    for(uint32_t i = 0; i < dsResult->resolutions.length; i++)
    {
        FFResolutionResult* result = ffListGet(&dsResult->resolutions, i);
        uint8_t moduleIndex = dsResult->resolutions.length == 1 ? 0 : (uint8_t) (i + 1);

        if(instance->config.resolution.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, FF_RESOLUTION_MODULE_NAME, moduleIndex, &instance->config.resolution.key);
            printf("%ix%i", result->width, result->height);

            if(result->refreshRate > 0)
                printf(" @ %iHz", result->refreshRate);

            if(result->brightness >= 0)
                printf(" (Brightness %d%%)", result->brightness);

            putchar('\n');
        }
        else
        {
            ffPrintFormat(instance, FF_RESOLUTION_MODULE_NAME, moduleIndex, &instance->config.resolution, FF_RESOLUTION_NUM_FORMAT_ARGS, (FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_INT, &result->width},
                {FF_FORMAT_ARG_TYPE_INT, &result->height},
                {FF_FORMAT_ARG_TYPE_INT, &result->refreshRate},
                {FF_FORMAT_ARG_TYPE_INT, &result->brightness},
            });
        }
    }

    if(dsResult->resolutions.length == 0)
        ffPrintError(instance, FF_RESOLUTION_MODULE_NAME, 0, &instance->config.resolution, "Couldn't detect resolution");
}
