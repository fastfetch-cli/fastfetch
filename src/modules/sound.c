#include "common/printing.h"
#include "detection/sound/sound.h"

#define FF_SOUND_MODULE_NAME "Sound"
#define FF_SOUND_NUM_FORMAT_ARGS 4

static void printDevice(FFinstance* instance, const FFSoundDevice* device, uint8_t index)
{
    if(instance->config.sound.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_SOUND_MODULE_NAME, index, &instance->config.sound.key);
        ffStrbufWriteTo(&device->name, stdout);

        if(device->volume > 0)
            printf(" (%d%%)", device->volume);
        else
            fputs(" (muted)", stdout);
        if(device->main && instance->config.soundShowAll)
            puts(" (*)");
        else
            putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_SOUND_MODULE_NAME, index, &instance->config.sound, FF_SOUND_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_BOOL, &device->main},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->name},
            {FF_FORMAT_ARG_TYPE_UINT8, &device->volume},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->manufacturer}
        });
    }
}

void ffPrintSound(FFinstance* instance)
{
    FF_LIST_AUTO_DESTROY result;
    ffListInit(&result, sizeof(FFSoundDevice));
    const char* error = ffDetectSound(instance, &result);

    if(error)
    {
        ffPrintError(instance, FF_SOUND_MODULE_NAME, 0, &instance->config.sound, "%s", error);
        return;
    }

    if(result.length == 0)
    {
        ffPrintError(instance, FF_SOUND_MODULE_NAME, 0, &instance->config.sound, "No sound devices found");
        return;
    }

    if(instance->config.soundShowAll)
    {
        uint8_t index = 0;
        FF_LIST_FOR_EACH(FFSoundDevice, device, result)
        {
            printDevice(instance, device, result.length == 1 ? 0 : ++index);
        }
    }
    else
    {
        FF_LIST_FOR_EACH(FFSoundDevice, device, result)
        {
            if (!device->main) continue;
            printDevice(instance, device, 0);
        }
    }
}
