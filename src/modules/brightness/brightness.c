#include "common/bar.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/brightness/brightness.h"
#include "modules/brightness/brightness.h"
#include "util/stringUtils.h"

#define FF_BRIGHTNESS_NUM_FORMAT_ARGS 2

void ffPrintBrightness(FFBrightnessOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFBrightnessResult));

    const char* error = ffDetectBrightness(&result);

    if(error)
    {
        ffPrintError(FF_BRIGHTNESS_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(result.length == 0)
    {
        ffPrintError(FF_BRIGHTNESS_MODULE_NAME, 0, &options->moduleArgs, "No result is detected.");
        return;
    }

    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    uint32_t index = 0;
    FF_LIST_FOR_EACH(FFBrightnessResult, item, result)
    {
        if(options->moduleArgs.key.length == 0)
        {
            ffStrbufAppendF(&key, "%s (%s)", FF_BRIGHTNESS_MODULE_NAME, item->name.chars);
        }
        else
        {
            uint32_t moduleIndex = result.length == 1 ? 0 : index + 1;
            ffParseFormatString(&key, &options->moduleArgs.key, 2, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_UINT, &moduleIndex},
                {FF_FORMAT_ARG_TYPE_STRBUF, &item->name}
            });
        }

        if(options->moduleArgs.outputFormat.length == 0)
        {
            FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();
            ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

            if (instance.config.percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            {
                ffAppendPercentBar(&str, item->value, 0, 100, 100);
            }

            if(instance.config.percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            {
                if(str.length > 0)
                    ffStrbufAppendC(&str, ' ');

                ffAppendPercentNum(&str, item->value, 10, 10, str.length > 0);
            }

            ffStrbufPutTo(&str, stdout);
        }
        else
        {
            FF_STRBUF_AUTO_DESTROY valueStr = ffStrbufCreate();
            ffAppendPercentNum(&valueStr, item->value, 10, 10, false);
            ffPrintFormatString(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, FF_BRIGHTNESS_NUM_FORMAT_ARGS, (FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_STRBUF, &valueStr},
                {FF_FORMAT_ARG_TYPE_STRBUF, &item->name},
            });
        }

        ffStrbufClear(&key);
        ffStrbufDestroy(&item->name);
        ++index;
    }
}

void ffInitBrightnessOptions(FFBrightnessOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_BRIGHTNESS_MODULE_NAME, ffParseBrightnessCommandOptions, ffParseBrightnessJsonObject, ffPrintBrightness, NULL);
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseBrightnessCommandOptions(FFBrightnessOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_BRIGHTNESS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyBrightnessOptions(FFBrightnessOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseBrightnessJsonObject(FFBrightnessOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_BRIGHTNESS_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}
