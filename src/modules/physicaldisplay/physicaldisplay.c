#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/physicaldisplay/physicaldisplay.h"
#include "modules/physicaldisplay/physicaldisplay.h"
#include "util/stringUtils.h"

#include <math.h>

#define FF_PHYSICALDISPLAY_DISPLAY_NAME "Physical Display"
#define FF_PHYSICALDISPLAY_NUM_FORMAT_ARGS 7

void ffPrintPhysicalDisplay(FFPhysicalDisplayOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFPhysicalDisplayResult));

    const char* error = ffDetectPhysicalDisplay(&result);

    if(error)
    {
        ffPrintError(FF_PHYSICALDISPLAY_DISPLAY_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(!result.length)
    {
        ffPrintError(FF_PHYSICALDISPLAY_DISPLAY_NAME, 0, &options->moduleArgs, "No physical display detected");
        return;
    }

    uint8_t index = 0;
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    FF_LIST_FOR_EACH(FFPhysicalDisplayResult, display, result)
    {
        double inch = sqrt(display->physicalWidth * display->physicalWidth + display->physicalHeight * display->physicalHeight) / 25.4;
        double ppi = sqrt(display->width * display->width + display->height * display->height) / inch;

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffStrbufClear(&key);
            if(options->moduleArgs.key.length == 0)
            {
                ffStrbufAppendF(&key, "%s (%s)", FF_PHYSICALDISPLAY_DISPLAY_NAME, display->name.chars);
            }
            else
            {
                ffParseFormatString(&key, &options->moduleArgs.key, 1, (FFformatarg[]){
                    {FF_FORMAT_ARG_TYPE_STRBUF, &display->name},
                });
            }
            ffPrintLogoAndKey(key.chars, 0, NULL, &options->moduleArgs.keyColor);

            printf("%ux%u px", display->width, display->height);
            if (inch > 0)
                printf(" - %ux%u mm (%.2f inches, %.2f ppi)\n", display->physicalWidth, display->physicalHeight, inch, ppi);
            else
                putchar('\n');
        }
        else
        {
            ffPrintFormat(FF_PHYSICALDISPLAY_DISPLAY_NAME, index, &options->moduleArgs, FF_PHYSICALDISPLAY_NUM_FORMAT_ARGS, (FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_STRBUF, &display->name},
                {FF_FORMAT_ARG_TYPE_UINT, &display->width},
                {FF_FORMAT_ARG_TYPE_UINT, &display->height},
                {FF_FORMAT_ARG_TYPE_UINT, &display->physicalWidth},
                {FF_FORMAT_ARG_TYPE_UINT, &display->physicalHeight},
                {FF_FORMAT_ARG_TYPE_DOUBLE, &inch},
                {FF_FORMAT_ARG_TYPE_DOUBLE, &ppi},
            });
        }

        ffStrbufDestroy(&display->name);
    }
}

void ffInitPhysicalDisplayOptions(FFPhysicalDisplayOptions* options)
{
    options->moduleName = FF_PHYSICALDISPLAY_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParsePhysicalDisplayCommandOptions(FFPhysicalDisplayOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_PHYSICALDISPLAY_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyPhysicalDisplayOptions(FFPhysicalDisplayOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParsePhysicalDisplayJsonObject(yyjson_val* module)
{
    FFPhysicalDisplayOptions __attribute__((__cleanup__(ffDestroyPhysicalDisplayOptions))) options;
    ffInitPhysicalDisplayOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(ffStrEqualsIgnCase(key, "type"))
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(FF_PHYSICALDISPLAY_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintPhysicalDisplay(&options);
}
