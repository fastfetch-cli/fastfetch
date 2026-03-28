extern "C"
{
#include "gpu.h"
#include "common/library.h"
#include "common/debug.h"
}

#if __has_include(<directx/dxcore.h>) && __has_include(<dxguids/dxguids.h>)

#include <directx/dxcore.h>
#include <dxguids/dxguids.h>
#include "common/windows/util.hpp"

static IDXCoreAdapterFactory* loadDxCoreFactory()
{
    static bool initialized = false;
    static IDXCoreAdapterFactory* factory = nullptr;
    if (initialized)
        return factory; // Already loaded

    initialized = true;
    FF_LIBRARY_LOAD(dxcore, NULL, "dxcore" FF_LIBRARY_EXTENSION, 1)

    // DXCoreCreateAdapterFactory is a reloaded function, so we can't use FF_LIBRARY_LOAD_SYMBOL_MESSAGE here
    typedef HRESULT (*DXCoreCreateAdapterFactory_t)(REFIID riid, void** ppvFactory);

    #ifndef FF_DISABLE_DLOPEN
        auto ffDXCoreCreateAdapterFactory = (DXCoreCreateAdapterFactory_t) dlsym(dxcore, "DXCoreCreateAdapterFactory");
        if (ffDXCoreCreateAdapterFactory == nullptr) return NULL;
    #else
        auto ffDXCoreCreateAdapterFactory = (DXCoreCreateAdapterFactory_t) DXCoreCreateAdapterFactory;
    #endif

    HRESULT hr = ffDXCoreCreateAdapterFactory(IID_PPV_ARGS(&factory));
    if (FAILED(hr))
    {
        FF_DEBUG("DXCoreCreateAdapterFactory failed with HRESULT: 0x%08lX (%s)", hr, ffDebugHResult(hr));
        return NULL;
    }

    dxcore = NULL; // Don't unload
    return factory;
}

extern "C"
const char* ffGPUDetectTypeWithDXCore(LUID adapterLuid, FFGPUResult* gpu)
{
    auto* factory = loadDxCoreFactory();
    if (!factory)
        return "Failed to load DXCore library or create adapter factory";

    IDXCoreAdapter *adapter = nullptr;
    HRESULT hr = factory->GetAdapterByLuid(adapterLuid, IID_PPV_ARGS(&adapter));
    if (FAILED(hr))
    {
        FF_DEBUG("GetAdapterByLuid failed with HRESULT: 0x%08lX (%s)", hr, ffDebugHResult(hr));
        return "Failed to get adapter by LUID";
    }

    on_scope_exit releaseAdapter {[adapter] { adapter->Release(); }};

    bool isIntegrated = false;
    hr = adapter->GetProperty(DXCoreAdapterProperty::IsIntegrated, sizeof(isIntegrated), &isIntegrated);
    if (FAILED(hr))
    {
        FF_DEBUG("GetProperty(IsIntegrated) failed with HRESULT: 0x%08lX (%s)", hr, ffDebugHResult(hr));
        return "Failed to get adapter properties";
    }

    gpu->type = isIntegrated ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;
    FF_DEBUG("GPU type determined using DXCore: %s", isIntegrated ? "Integrated" : "Discrete");

    return nullptr;
}

#else

#warning "DXCore headers not available, GPU type detection may be less accurate"

extern "C"
const char* ffGPUDetectTypeWithDXCore(LUID adapterLuid, FFGPUResult* gpu)
{
    FF_UNUSED(adapterLuid, gpu);
    FF_DEBUG("DXCore not available, skipping GPU type detection with DXCore");
    return "DXCore not available";
}

#endif
