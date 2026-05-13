extern "C" {
#include "sound.h"
#include "common/windows/com.h"
}
#include "common/windows/unicode.hpp"
#include "common/windows/variant.hpp"

#include <initguid.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>

static void ffCoTaskMemFreeWrapper(void* pptr) {
    assert(pptr != NULL);
    void* ptr = *(void**) pptr;
    if (ptr) {
        CoTaskMemFree(ptr);
    }
}
#define FF_COTASK_AUTO_FREE FF_A_CLEANUP(ffCoTaskMemFreeWrapper)

static const char* detectSoundDevice(FFlist* devices /* List of FFSoundDevice */, IMMDevice* immDevice, LPWSTR mainDeviceId) {
    LPWSTR FF_COTASK_AUTO_FREE immDeviceId = NULL;
    if (FAILED(immDevice->GetId(&immDeviceId))) {
        return "immDevice->GetId() failed";
    }

    IPropertyStore* FF_AUTO_RELEASE_COM_OBJECT immPropStore = NULL;
    if (FAILED(immDevice->OpenPropertyStore(STGM_READ, &immPropStore))) {
        return "immDevice->OpenPropertyStore() failed";
    }

    DWORD immState;
    if (FAILED(immDevice->GetState(&immState))) {
        return "immDevice->GetState() failed";
    }

    FFSoundDevice* device = FF_LIST_ADD(FFSoundDevice, *devices);
    device->type = (FFSoundType) ((!mainDeviceId || wcscmp(immDeviceId, mainDeviceId) == 0 ? FF_SOUND_TYPE_MAIN : FF_SOUND_TYPE_NONE) |
        ((immState & DEVICE_STATE_ACTIVE) ? FF_SOUND_TYPE_ACTIVE : FF_SOUND_TYPE_NONE));
    device->volume = FF_SOUND_VOLUME_UNKNOWN;
    ffStrbufInitWS(&device->identifier, immDeviceId);
    ffStrbufInit(&device->name);
    ffStrbufInitStatic(&device->platformApi, "Core Audio APIs");

    {
        FFPropVariant friendlyName;
        if (SUCCEEDED(immPropStore->GetValue(PKEY_Device_FriendlyName, &friendlyName))) {
            ffStrbufSetWSV(&device->name, friendlyName.get<std::wstring_view>());
        } else if (SUCCEEDED(immPropStore->GetValue(PKEY_Device_DeviceDesc, &friendlyName))) {
            ffStrbufSetWSV(&device->name, friendlyName.get<std::wstring_view>());
        } else {
            ffStrbufSetStatic(&device->name, "Unknown Device");
        }
    }

    IAudioEndpointVolume* FF_AUTO_RELEASE_COM_OBJECT immEndpointVolume = NULL;
    if (SUCCEEDED(immDevice->Activate(IID_IAudioEndpointVolume, CLSCTX_ALL, NULL, (void**) &immEndpointVolume))) {
        BOOL muted;
        if (FAILED(immEndpointVolume->GetMute(&muted)) || !muted) {
            FLOAT volume;
            if (SUCCEEDED(immEndpointVolume->GetMasterVolumeLevelScalar(&volume))) {
                device->volume = (uint8_t) (volume * 100 + 0.5);
            }
        }
    }

    return NULL;
}

const char* ffDetectSound(FFSoundOptions* options, FFlist* devices /* List of FFSoundDevice */) {
    const char* error = ffInitCom();
    if (error) {
        return error;
    }

    IMMDeviceEnumerator* FF_AUTO_RELEASE_COM_OBJECT pEnum = NULL;

    if (FAILED(CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pEnum)))) {
        return "CoCreateInstance(CLSID_MMDeviceEnumerator) failed";
    }

    LPWSTR FF_COTASK_AUTO_FREE mainDeviceId = NULL;

    {
        IMMDevice* FF_AUTO_RELEASE_COM_OBJECT pDefaultDevice = NULL;

        if (FAILED(pEnum->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDefaultDevice))) {
            return "GetDefaultAudioEndpoint() failed";
        }

        if (options->soundType & FF_SOUND_TYPE_MAIN) {
            return detectSoundDevice(devices, pDefaultDevice, NULL);
        }

        if (FAILED(pDefaultDevice->GetId(&mainDeviceId))) {
            return "pDefaultDevice->GetId() failed";
        }
    }

    IMMDeviceCollection* FF_AUTO_RELEASE_COM_OBJECT pDevices = NULL;

    if (FAILED(pEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE | (options->soundType & FF_SOUND_TYPE_ACTIVE ? 0 : DEVICE_STATE_DISABLED), &pDevices))) {
        return "EnumAudioEndpoints() failed";
    }

    uint32_t deviceCount;
    if (FAILED(pDevices->GetCount(&deviceCount))) {
        return "pDevices->GetCount() failed";
    }

    for (uint32_t deviceIdx = 0; deviceIdx < deviceCount; ++deviceIdx) {
        IMMDevice* FF_AUTO_RELEASE_COM_OBJECT immDevice = NULL;
        if (FAILED(pDevices->Item(deviceIdx, &immDevice))) {
            continue;
        }

        detectSoundDevice(devices, immDevice, mainDeviceId);
    }

    return NULL;
}
