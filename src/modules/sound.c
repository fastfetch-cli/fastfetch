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

        if(device->main && index > 0)
            fputs(" (*)", stdout);

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

static void printSound(FFinstance* instance, FFlist* devices)
{
    FF_LIST_AUTO_DESTROY filtered;
    ffListInit(&filtered, sizeof(FFSoundDevice*));

    FF_LIST_FOR_EACH(FFSoundDevice, device, *devices)
    {
        if(!device->active && !instance->config.soundShowAll)
            continue;

        *(FFSoundDevice**)ffListAdd(&filtered) = device;
    }

    if(filtered.length == 0)
    {
        ffPrintError(instance, FF_SOUND_MODULE_NAME, 0, &instance->config.sound, "No active sound devices found");
        return;
    }

    uint8_t index = 1;
    FF_LIST_FOR_EACH(FFSoundDevice*, device, filtered)
        printDevice(instance, *device, filtered.length == 1 ? 0 : index++);
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

    printSound(instance, &result);

    FF_LIST_FOR_EACH(FFSoundDevice, device, result)
    {
        ffStrbufDestroy(&device->name);
        ffStrbufDestroy(&device->manufacturer);
    }
}
