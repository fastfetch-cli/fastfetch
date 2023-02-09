extern "C" {
#include "gpu.h"
#include "util/windows/unicode.h"
#include "util/windows/registry.h"
}
#include "util/windows/wmi.hpp"

#include <dxgi.h>
#include <wchar.h>
#include <inttypes.h>

static const char* detectWithRegistry(FFlist* gpus)
{
    // Ref: https://github.com/lptstr/winfetch/pull/155

    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\DirectX", &hKey, nullptr))
        return "Open \"HKLM\\SOFTWARE\\Microsoft\\DirectX\" failed";

    uint64_t lastSeen;
    if(!ffRegReadUint64(hKey, L"LastSeen", &lastSeen, nullptr))
        return "Read \"HKLM\\SOFTWARE\\Microsoft\\DirectX\\LastSeen\" failed";

    DWORD index = 0;
    wchar_t subKeyName[64];
    while(true)
    {
        DWORD subKeySize = sizeof(subKeyName) / sizeof(*subKeyName);
        if(RegEnumKeyExW(hKey, index++, subKeyName, &subKeySize, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS)
            break;

        FF_HKEY_AUTO_DESTROY hSubKey = NULL;
        if(!ffRegOpenKeyForRead(hKey, subKeyName, &hSubKey, nullptr))
            continue;

        uint64_t adapterLastSeen;
        if(!ffRegReadUint64(hSubKey, L"LastSeen", &adapterLastSeen, nullptr) || lastSeen != adapterLastSeen)
            continue;

        uint32_t softwareAdapter;
        if(!ffRegReadUint(hSubKey, L"SoftwareAdapter", &softwareAdapter, nullptr) || softwareAdapter)
            continue;

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInit(&gpu->vendor);
        ffStrbufInit(&gpu->name);
        ffStrbufInit(&gpu->driver);
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;

        ffRegReadStrbuf(hSubKey, L"Description", &gpu->name, nullptr);

        uint32_t vendorId;
        if(ffRegReadUint(hSubKey, L"VendorId", &vendorId, nullptr))
            ffStrbufAppendS(&gpu->vendor, ffGetGPUVendorString(vendorId));

        uint64_t dedicatedVideoMemory;
        if(ffRegReadUint64(hSubKey, L"DedicatedVideoMemory", &dedicatedVideoMemory, nullptr))
            gpu->type = dedicatedVideoMemory >= 1024 * 1024 * 1024 ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;

        uint64_t driverVersion;
        if(ffRegReadUint64(hSubKey, L"DriverVersion", &driverVersion, nullptr))
        {
            ffStrbufSetF(&gpu->driver, "%" PRIu64 ".%" PRIu64 ".%" PRIu64 ".%" PRIu64,
                (driverVersion >> 48) & 0xFFFF,
                (driverVersion >> 32) & 0xFFFF,
                (driverVersion >> 16) & 0xFFFF,
                (driverVersion >>  0) & 0xFFFF);
        }
    }

    return nullptr;
}

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

        ffStrbufInit(&gpu->vendor);
        ffStrbufAppendS(&gpu->vendor, ffGetGPUVendorString(desc.VendorId));

        ffStrbufInit(&gpu->name);
        ffStrbufSetWS(&gpu->name, desc.Description);

        ffStrbufInit(&gpu->driver);

        gpu->type = desc.DedicatedVideoMemory >= 1024 * 1024 * 1024 ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;

        adapter->Release();

        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
    }

    pFactory->Release();

    return nullptr;
}

static const char* detectWithWmi(FFlist* gpus)
{
    FFWmiQuery query(L"SELECT Name, AdapterCompatibility, DriverVersion FROM Win32_VideoController", nullptr);
    if(!query)
        return "Query WMI service failed";

    while(FFWmiRecord record = query.next())
    {
        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);

        gpu->type = FF_GPU_TYPE_UNKNOWN;

        ffStrbufInit(&gpu->vendor);
        record.getString(L"AdapterCompatibility", &gpu->vendor);
        if(ffStrbufStartsWithS(&gpu->vendor, "Intel "))
        {
            //Intel returns "Intel Corporation", not sure about AMD
            ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
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

extern "C"
const char* ffDetectGPUImpl(FFlist* gpus, FF_MAYBE_UNUSED const FFinstance* instance)
{
    if (instance->config.allowSlowOperations)
        return detectWithWmi(gpus);

    if (!detectWithRegistry(gpus))
        return nullptr;
    return detectWithDxgi(gpus);
}
