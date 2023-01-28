#include "sound.h"
#include "util/apple/cf_helpers.h"

#include <CoreAudio/CoreAudio.h>

const char* ffDetectSound(FF_MAYBE_UNUSED const FFinstance* instance, FFlist* devices /* List of FFSoundDevice */)
{
    AudioDeviceID mainDeviceId;
    UInt32 dataSize = sizeof(mainDeviceId);
    if(AudioObjectGetPropertyData(kAudioObjectSystemObject, &(AudioObjectPropertyAddress){
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMain
    }, 0, NULL, &dataSize, &mainDeviceId) != kAudioHardwareNoError)
        return "AudioObjectGetPropertyData(kAudioHardwarePropertyDefaultOutputDevice) failed";

    AudioDeviceID deviceIds[32];
    dataSize = sizeof(deviceIds);
    if(AudioObjectGetPropertyData(kAudioObjectSystemObject, &(AudioObjectPropertyAddress){
        kAudioHardwarePropertyDevices,
        kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMain
    }, 0, NULL, &dataSize, &deviceIds) != kAudioHardwareNoError)
        return "AudioObjectGetPropertyData(kAudioHardwarePropertyDevices) failed";

    for(uint32_t index = 0, length = dataSize / sizeof(*deviceIds); index < length; ++index)
    {
        AudioDeviceID deviceId = deviceIds[index];

        uint32_t muted;
        dataSize = sizeof(muted);
        if(AudioObjectGetPropertyData(deviceId, &(AudioObjectPropertyAddress){
            kAudioDevicePropertyMute,
            kAudioDevicePropertyScopeOutput,
            0
        }, 0, NULL, &dataSize, &muted) != kAudioHardwareNoError)
            continue;

        FFSoundDevice* device = (FFSoundDevice*) ffListAdd(devices);
        device->main = deviceId == mainDeviceId;
        device->active = device->main;
        device->volume = 0;
        ffStrbufInitF(&device->identifier, "%u", (unsigned) deviceId);
        ffStrbufInit(&device->name);
        ffStrbufInit(&device->manufacturer);

        if (!muted)
        {
            float volume;
            dataSize = sizeof(volume);
            if(AudioObjectGetPropertyData(deviceId, &(AudioObjectPropertyAddress){
                kAudioDevicePropertyVolumeScalar,
                kAudioDevicePropertyScopeOutput,
                0
            }, 0, NULL, &dataSize, &volume) == kAudioHardwareNoError)
                device->volume = (uint8_t) (volume * 100 + 0.5);
        }

        CFStringRef name;
        dataSize = sizeof(name);
        if(AudioObjectGetPropertyData(deviceId, &(AudioObjectPropertyAddress){
            kAudioObjectPropertyName,
            kAudioDevicePropertyScopeOutput,
            0
        }, 0, NULL, &dataSize, &name) == kAudioHardwareNoError)
            ffCfStrGetString(name, &device->name);

        CFStringRef manufacturer;
        dataSize = sizeof(manufacturer);
        if(AudioObjectGetPropertyData(deviceId, &(AudioObjectPropertyAddress){
            kAudioObjectPropertyManufacturer,
            kAudioDevicePropertyScopeOutput,
            0
        }, 0, NULL, &dataSize, &manufacturer) == kAudioHardwareNoError)
            ffCfStrGetString(manufacturer, &device->manufacturer);
    }

    return NULL;
}
