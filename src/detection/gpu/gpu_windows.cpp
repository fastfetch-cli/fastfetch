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
        gpu->id = 0;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;

        ffRegReadUint64(hSubKey, L"AdapterLuid", &gpu->id, nullptr);

        ffRegReadStrbuf(hSubKey, L"Description", &gpu->name, nullptr);

        uint32_t vendorId;
        if(ffRegReadUint(hSubKey, L"VendorId", &vendorId, nullptr))
            ffStrbufAppendS(&gpu->vendor, ffGetGPUVendorString(vendorId));

        uint64_t dedicatedVideoMemory = 0;
        if(ffRegReadUint64(hSubKey, L"DedicatedVideoMemory", &dedicatedVideoMemory, nullptr))
            gpu->type = dedicatedVideoMemory >= 1024 * 1024 * 1024 ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;

        uint64_t dedicatedSystemMemory, sharedSystemMemory;
        if(ffRegReadUint64(hSubKey, L"DedicatedSystemMemory", &dedicatedSystemMemory, nullptr) &&
           ffRegReadUint64(hSubKey, L"SharedSystemMemory", &sharedSystemMemory, nullptr))
        {
            gpu->dedicated.total = dedicatedVideoMemory + dedicatedSystemMemory;
            gpu->shared.total = sharedSystemMemory;
        }

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
        gpu->dedicated.total = gpu->dedicated.total = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;

        ffStrbufInit(&gpu->vendor);
        ffStrbufAppendS(&gpu->vendor, ffGetGPUVendorString(desc.VendorId));

        gpu->id = (uint64_t&)desc.AdapterLuid;

        ffStrbufInit(&gpu->name);
        ffStrbufSetWS(&gpu->name, desc.Description);

        ffStrbufInit(&gpu->driver);

        gpu->type = desc.DedicatedVideoMemory >= 1024 * 1024 * 1024 ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;
        gpu->dedicated.total = desc.DedicatedVideoMemory + desc.DedicatedSystemMemory;
        gpu->shared.total = desc.SharedSystemMemory;

        adapter->Release();

        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
    }

    pFactory->Release();

    return nullptr;
}

static const char* detectMemoryUsage(FFlist* gpus)
{
    FFWmiQuery query(L"SELECT Name, DedicatedUsage, SharedUsage FROM Win32_PerfRawData_GPUPerformanceCounters_GPUAdapterMemory", nullptr);
    if(!query)
        return "Query WMI service failed";

    FF_STRBUF_AUTO_DESTROY name;
    ffStrbufInit(&name);

    while(FFWmiRecord record = query.next())
    {
        record.getString(L"Name", &name); // luid_0x00000000_0x000146E8_phys_0
        assert(name.length == strlen("luid_0x00000000_0x000146E8_phys_0"));
        ffStrbufSubstrBefore(&name, strlen("luid_0x00000000_0x000146E8")); // luid_0x00000000_0x000146E8
        ffStrbufSubstrAfter(&name, strlen("luid_0x00000000_0x") - 1); // 000146E8
        uint64_t luid = strtoull(name.chars, nullptr, 16);

        FF_LIST_FOR_EACH(FFGPUResult, gpu, *gpus)
        {
            if (gpu->id != luid) continue;
            record.getUnsigned(L"DedicatedUsage", &gpu->dedicated.used);
            record.getUnsigned(L"SharedUsage", &gpu->shared.used);
            break;
        }
    }

    return nullptr;
}

extern "C"
const char* ffDetectGPUImpl(FFlist* gpus, FF_MAYBE_UNUSED const FFinstance* instance)
{
    const char* error = detectWithRegistry(gpus);
    if (error)
        error = detectWithDxgi(gpus);

    if (!error && gpus->length > 0 && instance->config.allowSlowOperations)
        detectMemoryUsage(gpus);

    return error;
}
