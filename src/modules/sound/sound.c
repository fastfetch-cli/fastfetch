#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/sound/sound.h"
#include "modules/sound/sound.h"
#include "util/stringUtils.h"

#define FF_SOUND_NUM_FORMAT_ARGS 4

static void printDevice(FFSoundOptions* options, const FFSoundDevice* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_SOUND_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&device->name, stdout);

        if(device->volume != FF_SOUND_VOLUME_UNKNOWN)
        {
            if(device->volume > 0)
                printf(" (%d%%)", device->volume);
            else
                fputs(" (muted)", stdout);
        }

        if(device->main && index > 0)
            fputs(" (*)", stdout);

        putchar('\n');
    }
    else
    {
        ffPrintFormat(FF_SOUND_MODULE_NAME, index, &options->moduleArgs, FF_SOUND_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_BOOL, &device->main},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->name},
            {FF_FORMAT_ARG_TYPE_UINT8, &device->volume},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->identifier}
        });
    }
}

void ffPrintSound(FFSoundOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFSoundDevice));

    const char* error = ffDetectSound(&result);

    if(error)
    {
        ffPrintError(FF_SOUND_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
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
        ffPrintError(FF_SOUND_MODULE_NAME, 0, &options->moduleArgs, "No active sound devices found");
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
                ffPrintError(FF_SOUND_MODULE_NAME, 0, &options->moduleArgs, "Invalid %s value: %s", key, error);
            else
                options->soundType = (FFSoundType) value;
            continue;
        }

        ffPrintError(FF_SOUND_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
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

    if(result.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No active sound devices found");
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
    ffPrintModuleFormatHelp(FF_SOUND_MODULE_NAME, "{2} ({3}%)", FF_SOUND_NUM_FORMAT_ARGS, (const char* []) {
        "Is main sound device",
        "Device name",
        "Volume",
        "Identifier"
    });
}

void ffInitSoundOptions(FFSoundOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_SOUND_MODULE_NAME,
        ffParseSoundCommandOptions,
        ffParseSoundJsonObject,
        ffPrintSound,
        ffGenerateSoundJsonResult,
        ffPrintSoundHelpFormat,
        ffGenerateSoundJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);

    options->soundType = FF_SOUND_TYPE_MAIN;
}

void ffDestroySoundOptions(FFSoundOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
