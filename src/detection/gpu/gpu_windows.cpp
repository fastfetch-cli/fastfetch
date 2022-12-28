extern "C" {
#include "gpu.h"
#include "util/windows/unicode.h"
}

#ifdef FF_USE_WIN_GPU_DXGI

#include <dxgi.h>
#include <wchar.h>

static const char* detectWithDxgi(FFlist* gpus)
{
    IDXGIFactory1* pFactory;
    if(FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory))))
        return "CreateDXGIFactory1() failed";

    for(unsigned iAdapter = 0;; ++iAdapter)
    {
        IDXGIAdapter1* adapter;
        if(FAILED(pFactory->EnumAdapters1(iAdapter, &adapter)))
            break;

        DXGI_ADAPTER_DESC1 desc;
        if(FAILED(adapter->GetDesc1(&desc)) || (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
            continue;

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);

        if(wmemchr((const wchar_t[]) {0x1002, 0x1022}, (wchar_t)desc.VendorId, 2))
            ffStrbufInitS(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD);
        else if(wmemchr((const wchar_t[]) {0x03e7, 0x8086, 0x8087}, (wchar_t)desc.VendorId, 3))
            ffStrbufInitS(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
        else if(wmemchr((const wchar_t[]) {0x0955, 0x10de, 0x12d2}, (wchar_t)desc.VendorId, 3))
            ffStrbufInitS(&gpu->vendor, FF_GPU_VENDOR_NAME_NVIDIA);
        else
            ffStrbufInit(&gpu->vendor);

        ffStrbufInit(&gpu->name);
        ffStrbufSetWS(&gpu->name, desc.Description);

        ffStrbufInit(&gpu->driver);

        adapter->Release();

        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
    }

    pFactory->Release();

    return NULL;
}

#else

#include "util/windows/wmi.hpp"

static const char* detectWithWmi(FFlist* gpus)
{
    FFWmiQuery query(L"SELECT Name, AdapterCompatibility, DriverVersion FROM Win32_VideoController", nullptr);
    if(!query)
        return "Query WMI service failed";

    while(FFWmiRecord record = query.next())
    {
        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);

        ffStrbufInit(&gpu->vendor);
        record.getString(L"AdapterCompatibility", &gpu->vendor);
        if(ffStrbufStartsWithS(&gpu->vendor, "Intel "))
        {
            //Intel returns "Intel Corporation", not sure about AMD
            ffStrbufSetS(&gpu->vendor, "Intel");
        }

        ffStrbufInit(&gpu->name);
        record.getString(L"Name", &gpu->name);

        ffStrbufInit(&gpu->driver);
        record.getString(L"DriverVersion", &gpu->driver);

        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
    }

    return nullptr;
}

#endif

extern "C"
const char* ffDetectGPUImpl(FFlist* gpus, const FFinstance* instance)
{
    FF_UNUSED(instance);

    #ifdef FF_USE_WIN_GPU_DXGI
    return detectWithDxgi(gpus);
    #else
    return detectWithWmi(gpus);
    #endif
}
