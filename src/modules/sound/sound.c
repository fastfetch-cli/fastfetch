#include "common/percent.h"
#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/sound/sound.h"
#include "modules/sound/sound.h"
#include "util/stringUtils.h"

#define FF_SOUND_NUM_FORMAT_ARGS 5

static void printDevice(FFSoundOptions* options, const FFSoundDevice* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_SOUND_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();
        if (!(instance.config.display.percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
            ffStrbufAppend(&str, &device->name);

        if(device->volume != FF_SOUND_VOLUME_UNKNOWN)
        {
            if (instance.config.display.percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            {
                if (str.length)
                    ffStrbufAppendC(&str, ' ');

                ffPercentAppendBar(&str, device->volume, options->percent, &options->moduleArgs);
            }

            if (instance.config.display.percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            {
                if (str.length)
                    ffStrbufAppendC(&str, ' ');

                ffPercentAppendNum(&str, device->volume, options->percent, str.length > 0, &options->moduleArgs);
            }
        }

        if (!(instance.config.display.percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
        {
            if (device->main && index > 0)
                ffStrbufAppendS(&str, " (*)");
        }

        ffStrbufPutTo(&str, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY percentageNum = ffStrbufCreate();
        ffPercentAppendNum(&percentageNum, device->volume, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY percentageBar = ffStrbufCreate();
        ffPercentAppendBar(&percentageBar, device->volume, options->percent, &options->moduleArgs);

        FF_PRINT_FORMAT_CHECKED(FF_SOUND_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_SOUND_NUM_FORMAT_ARGS, ((FFformatarg[]) {
            FF_FORMAT_ARG(device->main, "is-main"),
            FF_FORMAT_ARG(device->name, "name"),
            FF_FORMAT_ARG(percentageNum, "volume-percentage"),
            FF_FORMAT_ARG(device->identifier, "identifier"),
            FF_FORMAT_ARG(percentageBar, "volume-percentage-bar"),
        }));
    }
}

void ffPrintSound(FFSoundOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFSoundDevice));

    const char* error = ffDetectSound(&result);

    if(error)
    {
        ffPrintError(FF_SOUND_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    FF_LIST_AUTO_DESTROY filtered = ffListCreate(sizeof(FFSoundDevice*));

    FF_LIST_FOR_EACH(FFSoundDevice, device, result)
    {
        switch (options->soundType)
        {
            case FF_SOUND_TYPE_MAIN: if (!device->main) continue; break;
            case FF_SOUND_TYPE_ACTIVE: if (!device->active) continue; break;
            case FF_SOUND_TYPE_ALL: break;
        }

        *(FFSoundDevice**)ffListAdd(&filtered) = device;
    }

    if(filtered.length == 0)
    {
        ffPrintError(FF_SOUND_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No active sound devices found");
        return;
    }

    uint8_t index = 1;
    FF_LIST_FOR_EACH(FFSoundDevice*, device, filtered)
    {
        printDevice(options, *device, filtered.length == 1 ? 0 : index++);
    }

    FF_LIST_FOR_EACH(FFSoundDevice, device, result)
    {
        ffStrbufDestroy(&device->identifier);
        ffStrbufDestroy(&device->name);
    }
}

bool ffParseSoundCommandOptions(FFSoundOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_SOUND_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "sound-type"))
    {
        options->soundType = (FFSoundType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "main", FF_SOUND_TYPE_MAIN },
            { "active", FF_SOUND_TYPE_ACTIVE },
            { "all", FF_SOUND_TYPE_ALL },
            {},
        });
        return true;
    }

    if (ffPercentParseCommandOptions(key, subKey, value, &options->percent))
        return true;

    return false;
}

void ffParseSoundJsonObject(FFSoundOptions* options, yyjson_val* module)
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

        if (ffStrEqualsIgnCase(key, "soundType"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "main", FF_SOUND_TYPE_MAIN },
                { "active", FF_SOUND_TYPE_ACTIVE },
                { "all", FF_SOUND_TYPE_ALL },
                {},
            });
            if (error)
                ffPrintError(FF_SOUND_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid %s value: %s", key, error);
            else
                options->soundType = (FFSoundType) value;
            continue;
        }

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_SOUND_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateSoundJsonConfig(FFSoundOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroySoundOptions))) FFSoundOptions defaultOptions;
    ffInitSoundOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (defaultOptions.soundType != options->soundType)
    {
        switch (options->soundType)
        {
            case FF_SOUND_TYPE_MAIN:
                yyjson_mut_obj_add_str(doc, module, "soundType", "main");
                break;
            case FF_SOUND_TYPE_ACTIVE:
                yyjson_mut_obj_add_str(doc, module, "soundType", "active");
                break;
            case FF_SOUND_TYPE_ALL:
                yyjson_mut_obj_add_str(doc, module, "soundType", "all");
                break;
        }
    }

    ffPercentGenerateJsonConfig(doc, module, defaultOptions.percent, options->percent);
}

void ffGenerateSoundJsonResult(FF_MAYBE_UNUSED FFSoundOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFSoundDevice));
    const char* error = ffDetectSound(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFSoundDevice, item, result)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_bool(doc, obj, "active", item->active);
        yyjson_mut_obj_add_bool(doc, obj, "main", item->main);

        if (item->volume != FF_SOUND_VOLUME_UNKNOWN)
            yyjson_mut_obj_add_uint(doc, obj, "volume", item->volume);
        else
            yyjson_mut_obj_add_null(doc, obj, "volume");

        yyjson_mut_obj_add_strbuf(doc, obj, "name", &item->name);
        yyjson_mut_obj_add_strbuf(doc, obj, "identifier", &item->identifier);
    }

    FF_LIST_FOR_EACH(FFSoundDevice, device, result)
    {
        ffStrbufDestroy(&device->identifier);
        ffStrbufDestroy(&device->name);
    }
}

void ffPrintSoundHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_SOUND_MODULE_NAME, "{2} ({3}%)", FF_SOUND_NUM_FORMAT_ARGS, ((const char* []) {
        "Is main sound device - is-main",
        "Device name - name",
        "Volume (in percentage num) - volume-percentage",
        "Identifier - identifier",
        "Volume (in percentage bar) - volume-percentage-bar",
    }));
}

void ffInitSoundOptions(FFSoundOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_SOUND_MODULE_NAME,
        "Print sound devices, volume, etc",
        ffParseSoundCommandOptions,
        ffParseSoundJsonObject,
        ffPrintSound,
        ffGenerateSoundJsonResult,
        ffPrintSoundHelpFormat,
        ffGenerateSoundJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "");

    options->soundType = FF_SOUND_TYPE_MAIN;
    options->percent = (FFColorRangeConfig) { 80, 90 };
}

void ffDestroySoundOptions(FFSoundOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
