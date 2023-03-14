#include "fastfetch.h"
#include "common/printing.h"
#include "detection/brightness/brightness.h"
#include "modules/brightness/brightness.h"

#define FF_BRIGHTNESS_MODULE_NAME "Brightness"
#define FF_BRIGHTNESS_NUM_FORMAT_ARGS 2

void ffPrintBrightness(FFinstance* instance, FFBrightnessOptions* options)
{
    FF_LIST_AUTO_DESTROY result;
    ffListInit(&result, sizeof(FFBrightnessResult));
    const char* error = ffDetectBrightness(&result);

    if(error)
    {
        ffPrintError(instance, FF_BRIGHTNESS_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(result.length == 0)
    {
        ffPrintError(instance, FF_BRIGHTNESS_MODULE_NAME, 0, &options->moduleArgs, "No result is detected.");
        return;
    }

    FF_STRBUF_AUTO_DESTROY key;
    ffStrbufInit(&key);

    FF_LIST_FOR_EACH(FFBrightnessResult, item, result)
    {
        if(options->moduleArgs.key.length == 0)
        {
            ffStrbufAppendF(&key, "%s (%s)", FF_BRIGHTNESS_MODULE_NAME, item->name.chars);
        }
        else
        {
            ffParseFormatString(&key, &options->moduleArgs.key, 1, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &item->name}
            });
        }

        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, key.chars, 0, NULL);
            printf("%.0f%%\n", item->value);
        }
        else
        {
            ffPrintFormatString(instance, key.chars, 0, NULL, &options->moduleArgs.outputFormat, FF_BRIGHTNESS_NUM_FORMAT_ARGS, (FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_STRBUF, &item->value},
                {FF_FORMAT_ARG_TYPE_FLOAT, &item->name}
            });
        }

        ffStrbufClear(&key);
        ffStrbufDestroy(&item->name);
    }
}

void ffInitBrightnessOptions(FFBrightnessOptions* options)
{
    options->moduleName = FF_BRIGHTNESS_MODULE_NAME;
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

#ifdef FF_HAVE_JSONC
bool ffParseBrightnessJsonObject(FFinstance* instance, const char* type, JSONCData* data, json_object* module)
{
    if (strcasecmp(type, FF_BRIGHTNESS_MODULE_NAME) != 0)
        return false;

    FFBrightnessOptions __attribute__((__cleanup__(ffDestroyBrightnessOptions))) options;
    ffInitBrightnessOptions(&options);

    if (module)
    {
        struct lh_entry* entry;
        lh_foreach(data->ffjson_object_get_object(module), entry)
        {
            const char* key = (const char *)lh_entry_k(entry);
            if (strcasecmp(key, "type") == 0)
                continue;
            json_object* val = (struct json_object *)lh_entry_v(entry);

            if (ffJsonConfigParseModuleArgs(data, key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_BRIGHTNESS_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintBrightness(instance, &options);
    return true;
}
#endif
