#include "common/printing.h"
#include "detection/sound/sound.h"
#include "modules/sound/sound.h"

#define FF_SOUND_NUM_FORMAT_ARGS 4

static void printDevice(FFinstance* instance, FFSoundOptions* options, const FFSoundDevice* device, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_SOUND_MODULE_NAME, index, &options->moduleArgs.key);
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
        ffPrintFormat(instance, FF_SOUND_MODULE_NAME, index, &options->moduleArgs, FF_SOUND_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_BOOL, &device->main},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->name},
            {FF_FORMAT_ARG_TYPE_UINT8, &device->volume},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->identifier}
        });
    }
}

void ffPrintSound(FFinstance* instance, FFSoundOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFSoundDevice));

    const char* error = ffDetectSound(instance, &result);

    if(error)
    {
        ffPrintError(instance, FF_SOUND_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
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
        ffPrintError(instance, FF_SOUND_MODULE_NAME, 0, &options->moduleArgs, "No active sound devices found");
        return;
    }

    uint8_t index = 1;
    FF_LIST_FOR_EACH(FFSoundDevice*, device, filtered)
    {
        printDevice(instance, options, *device, filtered.length == 1 ? 0 : index++);
    }

    FF_LIST_FOR_EACH(FFSoundDevice, device, result)
    {
        ffStrbufDestroy(&device->identifier);
        ffStrbufDestroy(&device->name);
    }
}


void ffInitSoundOptions(FFSoundOptions* options)
{
    options->moduleName = FF_SOUND_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);

    options->soundType = FF_SOUND_TYPE_MAIN;
}

bool ffParseSoundCommandOptions(FFSoundOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_SOUND_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (strcasecmp(subKey, "sound-type") == 0)
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

void ffDestroySoundOptions(FFSoundOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

#ifdef FF_HAVE_JSONC
void ffParseSoundJsonObject(FFinstance* instance, json_object* module)
{
    FFSoundOptions __attribute__((__cleanup__(ffDestroySoundOptions))) options;
    ffInitSoundOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            if (strcasecmp(key, "soundType") == 0)
            {
                int value;
                const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                    { "main", FF_SOUND_TYPE_MAIN },
                    { "active", FF_SOUND_TYPE_ACTIVE },
                    { "all", FF_SOUND_TYPE_ALL },
                    {},
                });
                if (error)
                    ffPrintError(instance, FF_SOUND_MODULE_NAME, 0, &options.moduleArgs, "Invalid %s value: %s", key, error);
                else
                    options.soundType = (FFSoundType) value;
                continue;
            }

            ffPrintError(instance, FF_SOUND_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintSound(instance, &options);
}
#endif
