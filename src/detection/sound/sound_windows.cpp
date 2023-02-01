extern "C" {
    #include "sound.h"
    #include "util/windows/unicode.h"
}
#include "util/windows/com.hpp"

#include <initguid.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>

const char* ffDetectSound(FF_MAYBE_UNUSED const FFinstance* instance, FF_MAYBE_UNUSED FFlist* devices /* List of FFSoundDevice */)
{
    const char* error = ffInitCom();
    if (error)
        return error;

    IMMDeviceEnumerator* FF_AUTO_RELEASE_COM_OBJECT pEnum = NULL;

    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **)&pEnum)))
        return "CoCreateInstance(__uuidof(MMDeviceEnumerator)) failed";

    LPWSTR mainDeviceId = NULL;

    {
        IMMDevice* FF_AUTO_RELEASE_COM_OBJECT pDefaultDevice = NULL;

        if (FAILED(pEnum->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDefaultDevice)))
            return "GetDefaultAudioEndpoint() failed";

        if (FAILED(pDefaultDevice->GetId(&mainDeviceId)))
            return "pDefaultDevice->GetId() failed";
    }

    IMMDeviceCollection* FF_AUTO_RELEASE_COM_OBJECT pDevices = NULL;

    if (FAILED(pEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED, &pDevices)))
        return "EnumAudioEndpoints() failed";

    uint32_t deviceCount;
    if (FAILED(pDevices->GetCount(&deviceCount)))
        return "pDevices->GetCount() failed";

    for (uint32_t deviceIdx = 0; deviceIdx < deviceCount; ++deviceIdx)
    {
        IMMDevice* FF_AUTO_RELEASE_COM_OBJECT immDevice = NULL;
        if (FAILED(pDevices->Item(deviceIdx, &immDevice)))
            continue;

        LPWSTR immDeviceId = NULL;
        if (FAILED(immDevice->GetId(&immDeviceId)))
            continue;

        IPropertyStore* FF_AUTO_RELEASE_COM_OBJECT immPropStore;
        if (FAILED(immDevice->OpenPropertyStore(STGM_READ, &immPropStore)))
            continue;

        DWORD immState;
        if (FAILED(immDevice->GetState(&immState)))
            continue;

        FFSoundDevice* device = (FFSoundDevice*) ffListAdd(devices);
        device->main = wcscmp(mainDeviceId, immDeviceId) == 0;
        device->active = !!(immState & DEVICE_STATE_ACTIVE);
        device->volume = 0;
        ffStrbufInit(&device->identifier);
        ffStrbufInit(&device->name);

        ffStrbufSetWS(&device->identifier, immDeviceId);

        {
            PROPVARIANT __attribute__((__cleanup__(PropVariantClear))) friendlyName;
            PropVariantInit(&friendlyName);
            if (SUCCEEDED(immPropStore->GetValue(PKEY_Device_FriendlyName, &friendlyName)))
                ffStrbufSetWS(&device->name, friendlyName.pwszVal);
        }

        IAudioEndpointVolume* FF_AUTO_RELEASE_COM_OBJECT immEndpointVolume;
        if(SUCCEEDED(immDevice->Activate(IID_IAudioEndpointVolume, CLSCTX_ALL, NULL, (void**) &immEndpointVolume)))
        {
            BOOL muted;
            if (FAILED(immEndpointVolume->GetMute(&muted)) || !muted)
            {
                FLOAT volume;
                if (SUCCEEDED(immEndpointVolume->GetMasterVolumeLevelScalar(&volume)))
                    device->volume = (uint8_t) (volume * 100 + 0.5);
            }
        }
    }

    return NULL;
}
