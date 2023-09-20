#ifdef FF_HAVE_DIRECTX_HEADERS

#define INITGUID
extern "C" {
#include "common/library.h"
#include "detection/gpu/gpu.h"
}

#include <wsl/winadapter.h>
#include <directx/dxcore.h>
#include <directx/d3d12.h>
#include <dxguids/dxguids.h>
#include <utility>
#include <cinttypes>

template <typename Fn>
struct on_scope_exit {
    on_scope_exit(Fn &&fn): _fn(std::move(fn)) {}
    ~on_scope_exit() { this->_fn(); }

private:
    Fn _fn;
};

extern "C"
const char* ffGPUDetectByDirectX(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    FF_LIBRARY_LOAD(libdxcore, nullptr, "dlopen libdxcore.so failed. Make sure WSLg is enabled", "/usr/lib/wsl/lib/libdxcore" FF_LIBRARY_EXTENSION, 4)
    // DXCoreCreateAdapterFactory is a reloaded function, so we can't use FF_LIBRARY_LOAD_SYMBOL_MESSAGE here
    typedef HRESULT (*DXCoreCreateAdapterFactory_t)(REFIID riid, void** ppvFactory);
    auto ffDXCoreCreateAdapterFactory = (DXCoreCreateAdapterFactory_t) dlsym(libdxcore, "DXCoreCreateAdapterFactory");
    if(ffDXCoreCreateAdapterFactory == nullptr) return "dlsym DXCoreCreateAdapterFactory failed";

    IDXCoreAdapterFactory *factory = nullptr;

    if (FAILED(ffDXCoreCreateAdapterFactory(IID_PPV_ARGS(&factory))))
        return "DXCoreCreateAdapterFactory(IID_PPV_ARGS(&factory)) failed";
    on_scope_exit destroyFactory([&] { factory->Release(); });

    IDXCoreAdapterList *list = NULL;
    if (FAILED(factory->CreateAdapterList(1, &DXCORE_ADAPTER_ATTRIBUTE_D3D11_GRAPHICS, &list)))
        return "factory->CreateAdapterList(1, &DXCORE_ADAPTER_ATTRIBUTE_D3D11_GRAPHICS, &list) failed";
    on_scope_exit destroyList([&] { list->Release(); });

    for (uint32_t i = 0, count = list->GetAdapterCount(); i < count; i++)
    {
        IDXCoreAdapter *adapter = nullptr;
        if (FAILED(list->GetAdapter(i, &adapter)))
            continue;
        on_scope_exit destroyAdapter([&] { adapter->Release(); });

        // https://learn.microsoft.com/en-us/windows/win32/api/dxcore_interface/ne-dxcore_interface-dxcoreadapterproperty
        {
            bool value = 0;
            if (SUCCEEDED(adapter->GetProperty(DXCoreAdapterProperty::IsHardware, sizeof(value), &value)) && !value)
                continue;
        }

        char desc[256];
        if (FAILED(adapter->GetProperty(DXCoreAdapterProperty::DriverDescription, sizeof(desc), desc)))
            continue;

        FFGPUResult* gpu = (FFGPUResult*) ffListAdd(gpus);
        ffStrbufInitS(&gpu->name, desc);
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->temperature = FF_GPU_TEMP_UNSET;

        ffStrbufInit(&gpu->driver);
        uint64_t value = 0;
        if (SUCCEEDED(adapter->GetProperty(DXCoreAdapterProperty::DriverVersion, sizeof(value), &value)))
        {
            ffStrbufSetF(&gpu->driver, "%" PRIu64 ".%" PRIu64 ".%" PRIu64 ".%" PRIu64,
                (value >> 48) & 0xFFFF,
                (value >> 32) & 0xFFFF,
                (value >> 16) & 0xFFFF,
                (value >>  0) & 0xFFFF);
        }

        gpu->dedicated.used = gpu->shared.used = gpu->dedicated.total = gpu->shared.total = FF_GPU_VMEM_SIZE_UNSET;
        if (SUCCEEDED(adapter->GetProperty(DXCoreAdapterProperty::DedicatedAdapterMemory, sizeof(value), &value)))
            gpu->dedicated.total = value;
        if (SUCCEEDED(adapter->GetProperty(DXCoreAdapterProperty::SharedSystemMemory, sizeof(value), &value)))
            gpu->shared.total = value;

        gpu->type = FF_GPU_TYPE_UNKNOWN;
        bool integrated;
        if (SUCCEEDED(adapter->GetProperty(DXCoreAdapterProperty::IsIntegrated, sizeof(integrated), &integrated)))
            gpu->type = integrated ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;

        ffStrbufInit(&gpu->vendor);
        DXCoreHardwareID hardwareId;
        if (SUCCEEDED(adapter->GetProperty(DXCoreAdapterProperty::HardwareID, sizeof(hardwareId), &hardwareId)))
        {
            const char* vendorStr = ffGetGPUVendorString((unsigned) hardwareId.vendorID);
            ffStrbufAppendS(&gpu->vendor, vendorStr);
        }
    }

    return NULL;
}

#endif
