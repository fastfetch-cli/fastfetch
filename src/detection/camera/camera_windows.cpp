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
    // TODO: check if supported by Windows Server Core

    const char* error = ffInitCom();
    if (error)
        return error;

    IMFAttributes* FF_AUTO_RELEASE_COM_OBJECT attrs = nullptr;
    if (FAILED(MFCreateAttributes(&attrs, 1)))
        return "MFCreateAttributes() failed";

    if (FAILED(attrs->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
    )))
        return "SetGUID(MF_*) failed";

    IMFActivate** devices = NULL;
    uint32_t count;

    if (FAILED(MFEnumDeviceSources(attrs, &devices, &count)))
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

        // Assume first type is the maximum resolution
        IMFMediaType* FF_AUTO_RELEASE_COM_OBJECT type = NULL;
        if (FAILED(handler->GetMediaTypeByIndex(0, &type)))
            continue;

        MFGetAttributeSize(type, MF_MT_FRAME_SIZE, &camera->width, &camera->height);
    }

    CoTaskMemFree(devices);

    return nullptr;
}
