extern "C" {
#include "camera.h"
#include "common/library.h"
}
#include "util/windows/com.hpp"
#include "util/windows/unicode.hpp"

#include <initguid.h>
#include <mfapi.h>
#include <mfidl.h>

template <typename Fn>
struct on_scope_exit {
    on_scope_exit(Fn &&fn): _fn(std::move(fn)) {}
    ~on_scope_exit() { this->_fn(); }

private:
    Fn _fn;
};

extern "C"
const char* ffDetectCamera(FF_MAYBE_UNUSED FFlist* result)
{
    FF_LIBRARY_LOAD(mfplat, "dlopen mfplat" FF_LIBRARY_EXTENSION " failed", "mfplat" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(mfplat, MFCreateAttributes)
    FF_LIBRARY_LOAD(mf, "dlopen mf" FF_LIBRARY_EXTENSION " failed", "mf" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(mf, MFEnumDeviceSources)

    const char* error = ffInitCom();
    if (error)
        return error;

    IMFAttributes* FF_AUTO_RELEASE_COM_OBJECT attrs = nullptr;
    if (FAILED(ffMFCreateAttributes(&attrs, 1)))
        return "MFCreateAttributes() failed";

    if (FAILED(attrs->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
    )))
        return "SetGUID(MF_*) failed";

    IMFActivate** devices = NULL;
    uint32_t count;

    if (FAILED(ffMFEnumDeviceSources(attrs, &devices, &count)))
        return "MFEnumDeviceSources() failed";

    for (uint32_t i = 0; i < count; i++)
    {
        IMFActivate* device = devices[i];

        wchar_t buffer[256];
        uint32_t length = 0;
        if (FAILED(device->GetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, buffer, sizeof(buffer), &length)) || length == 0)
            continue;

        FFCameraResult* camera = (FFCameraResult*) ffListAdd(result);
        ffStrbufInitNWS(&camera->name, length, buffer);
        ffStrbufInit(&camera->colorspace);
        ffStrbufInit(&camera->vendor);
        ffStrbufInit(&camera->id);
        camera->width = 0;
        camera->height = 0;

        if (SUCCEEDED(device->GetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, buffer, sizeof(buffer), &length)) && length > 0)
            ffStrbufSetNWS(&camera->id, length, buffer);

        IMFMediaSource* FF_AUTO_RELEASE_COM_OBJECT source = nullptr;
        if (FAILED(device->ActivateObject(IID_PPV_ARGS(&source))))
            continue;

        on_scope_exit destroySource([&] { source->Shutdown(); });

        IMFPresentationDescriptor* FF_AUTO_RELEASE_COM_OBJECT pd = nullptr;
        if (FAILED(source->CreatePresentationDescriptor(&pd)))
            continue;

        IMFStreamDescriptor* FF_AUTO_RELEASE_COM_OBJECT sd = NULL;
        BOOL selected;
        if (FAILED(pd->GetStreamDescriptorByIndex(0, &selected, &sd)))
            continue;

        IMFMediaTypeHandler* FF_AUTO_RELEASE_COM_OBJECT handler = NULL;
        if (FAILED(sd->GetMediaTypeHandler(&handler)))
            continue;

        DWORD mediaTypeCount;
        if (FAILED(handler->GetMediaTypeCount(&mediaTypeCount)))
            continue;

        // Assume first type is the maximum resolution
        IMFMediaType* FF_AUTO_RELEASE_COM_OBJECT type = NULL;
        for (DWORD idx = 0; SUCCEEDED(handler->GetMediaTypeByIndex(idx, &type)); ++idx)
        {
            GUID majorType;
            if (FAILED(type->GetMajorType(&majorType)) || majorType != MFMediaType_Video)
                continue;

            MFVideoPrimaries primaries;
            static_assert(sizeof(primaries) == sizeof(uint32_t), "");
            if (SUCCEEDED(type->GetUINT32(MF_MT_VIDEO_PRIMARIES, (uint32_t*) &primaries)))
            {
                switch (primaries)
                {
                case MFVideoPrimaries_BT709: ffStrbufSetStatic(&camera->colorspace, "sRGB"); break;
                case MFVideoPrimaries_BT470_2_SysM:
                case MFVideoPrimaries_BT470_2_SysBG: ffStrbufSetStatic(&camera->colorspace, "NTSC"); break;
                case MFVideoPrimaries_SMPTE170M: ffStrbufSetStatic(&camera->colorspace, "SMPTE 170M"); break;
                case MFVideoPrimaries_SMPTE240M: ffStrbufSetStatic(&camera->colorspace, "SMPTE 240M"); break;
                case MFVideoPrimaries_EBU3213: ffStrbufSetStatic(&camera->colorspace, "EBU 3213"); break;
                case MFVideoPrimaries_SMPTE_C: ffStrbufSetStatic(&camera->colorspace, "SMPTE C"); break;
                case MFVideoPrimaries_BT2020: ffStrbufSetStatic(&camera->colorspace, "BT.2020"); break;
                case MFVideoPrimaries_XYZ: ffStrbufSetStatic(&camera->colorspace, "XYZ"); break;
                case MFVideoPrimaries_DCI_P3: ffStrbufSetStatic(&camera->colorspace, "DCI-P3"); break;
                case MFVideoPrimaries_ACES: ffStrbufSetStatic(&camera->colorspace, "ACES"); break;
                default: break;
                }
            }

            MFGetAttributeSize(type, MF_MT_FRAME_SIZE, &camera->width, &camera->height);
            break;
        }
    }

    CoTaskMemFree(devices);

    return nullptr;
}
