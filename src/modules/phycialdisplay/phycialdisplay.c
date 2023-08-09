#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/phycialdisplay/phycialdisplay.h"
#include "modules/phycialdisplay/phycialdisplay.h"
#include "util/stringUtils.h"

#define FF_PHYCIALDISPLAY_DISPLAY_NAME "Phycial Display"
#define FF_PHYCIALDISPLAY_NUM_FORMAT_ARGS 3

void ffPrintPhycialDisplay(FFPhycialDisplayOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFPhycialDisplayResult));

    const char* error = ffDetectPhycialDisplay(&result);

    if(error)
    {
        ffPrintError(FF_PHYCIALDISPLAY_DISPLAY_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(!result.length)
    {
        ffPrintError(FF_PHYCIALDISPLAY_DISPLAY_NAME, 0, &options->moduleArgs, "No phycial display detected");
        return;
    }

    uint8_t index = 0;
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    FF_LIST_FOR_EACH(FFPhycialDisplayResult, display, result)
    {
        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffStrbufClear(&key);
            if(options->moduleArgs.key.length == 0)
            {
                ffStrbufAppendF(&key, "%s (%s)", FF_PHYCIALDISPLAY_DISPLAY_NAME, display->name.chars);
            }
            else
            {
                ffParseFormatString(&key, &options->moduleArgs.key, 1, (FFformatarg[]){
                    {FF_FORMAT_ARG_TYPE_STRBUF, &display->name},
                });
            }
            ffPrintLogoAndKey(key.chars, 0, NULL, &options->moduleArgs.keyColor);

            printf("%ux%u\n", display->width, display->height);
        }
        else
        {
            ffPrintFormat(FF_PHYCIALDISPLAY_DISPLAY_NAME, index, &options->moduleArgs, FF_PHYCIALDISPLAY_NUM_FORMAT_ARGS, (FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_STRBUF, &display->name},
                {FF_FORMAT_ARG_TYPE_UINT, &display->width},
                {FF_FORMAT_ARG_TYPE_UINT, &display->height},
            });
        }

        ffStrbufDestroy(&display->name);
    }
}

void ffInitPhycialDisplayOptions(FFPhycialDisplayOptions* options)
{
    options->moduleName = FF_PHYCIALDISPLAY_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParsePhycialDisplayCommandOptions(FFPhycialDisplayOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_PHYCIALDISPLAY_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyPhycialDisplayOptions(FFPhycialDisplayOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParsePhycialDisplayJsonObject(yyjson_val* module)
{
    FFPhycialDisplayOptions __attribute__((__cleanup__(ffDestroyPhycialDisplayOptions))) options;
    ffInitPhycialDisplayOptions(&options);

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

            ffPrintError(FF_PHYCIALDISPLAY_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintPhycialDisplay(&options);
}
