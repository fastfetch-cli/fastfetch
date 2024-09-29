#include "gpu.h"
#include "detection/gpu/gpu_driver_specific.h"
#include "util/windows/unicode.h"
#include "util/windows/registry.h"

#define INITGUID
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>

static inline void wrapSetupDiDestroyDeviceInfoList(HDEVINFO* hdev)
{
    if(*hdev)
        SetupDiDestroyDeviceInfoList(*hdev);
}

#define FF_EMPTY_GUID_STR L"{00000000-0000-0000-0000-000000000000}"
enum { FF_GUID_STRLEN = sizeof(FF_EMPTY_GUID_STR) / sizeof(wchar_t) - 1 };

wchar_t regDirectxKey[] = L"SOFTWARE\\Microsoft\\DirectX\\" FF_EMPTY_GUID_STR;
const uint32_t regDirectxKeyPrefixLength = (uint32_t) __builtin_strlen("SOFTWARE\\Microsoft\\DirectX\\");
wchar_t regDriverKey[] = L"SYSTEM\\CurrentControlSet\\Control\\Class\\" FF_EMPTY_GUID_STR L"\\0000";
const uint32_t regDriverKeyPrefixLength = (uint32_t) __builtin_strlen("SYSTEM\\CurrentControlSet\\Control\\Class\\");

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    HDEVINFO hdev __attribute__((__cleanup__(wrapSetupDiDestroyDeviceInfoList))) =
        SetupDiGetClassDevsW(&GUID_DEVCLASS_DISPLAY, NULL, NULL, DIGCF_PRESENT);

    if(hdev == INVALID_HANDLE_VALUE)
        return "SetupDiGetClassDevsW(&GUID_DEVCLASS_DISPLAY) failed";

    SP_DEVINFO_DATA did = { .cbSize = sizeof(did) };
    for (DWORD idev = 0; SetupDiEnumDeviceInfo(hdev, idev, &did); ++idev)
    {
        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInit(&gpu->vendor);
        ffStrbufInit(&gpu->name);
        ffStrbufInit(&gpu->driver);
        ffStrbufInitStatic(&gpu->platformApi, "SetupAPI");
        gpu->index = FF_GPU_INDEX_UNSET;
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = 0;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        uint32_t pciBus = 0, pciAddr = UINT32_MAX, pciDev = 0, pciFunc = 0;
        if (SetupDiGetDeviceRegistryPropertyW(hdev, &did, SPDRP_BUSNUMBER, NULL, (PBYTE) &pciBus, sizeof(pciBus), NULL) &&
            SetupDiGetDeviceRegistryPropertyW(hdev, &did, SPDRP_ADDRESS, NULL, (PBYTE) &pciAddr, sizeof(pciAddr), NULL))
        {
            pciDev = (pciAddr >> 16) & 0xFFFF;
            pciFunc = pciAddr & 0xFFFF;
            gpu->deviceId = (pciBus * 1000ull) + (pciDev * 10ull) + pciFunc;
        }

        wchar_t buffer[256];

        uint64_t adapterLuid = 0;

        FF_HKEY_AUTO_DESTROY hVideoIdKey = SetupDiOpenDevRegKey(hdev, &did, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
        if (!hVideoIdKey) continue;
        DWORD bufferLen = sizeof(buffer);
        if (RegGetValueW(hVideoIdKey, NULL, L"VideoID", RRF_RT_REG_SZ, NULL, buffer, &bufferLen) == ERROR_SUCCESS &&
            bufferLen == (FF_GUID_STRLEN + 1) * sizeof(wchar_t))
        {
            wmemcpy(regDirectxKey + regDirectxKeyPrefixLength, buffer, FF_GUID_STRLEN);
            FF_HKEY_AUTO_DESTROY hDirectxKey = NULL;
            if (ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regDirectxKey, &hDirectxKey, NULL))
            {
                uint32_t vendorId = 0;
                if(ffRegReadUint(hDirectxKey, L"VendorId", &vendorId, NULL) && vendorId)
                    ffStrbufSetStatic(&gpu->vendor, ffGetGPUVendorString(vendorId));

                if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_INTEL)
                    gpu->type = gpu->deviceId == 20 ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;

                uint64_t dedicatedVideoMemory = 0;
                if(ffRegReadUint64(hDirectxKey, L"DedicatedVideoMemory", &dedicatedVideoMemory, NULL))
                {
                    if (gpu->type == FF_GPU_TYPE_UNKNOWN)
                        gpu->type = dedicatedVideoMemory >= 1024 * 1024 * 1024 ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;
                }

                uint64_t dedicatedSystemMemory, sharedSystemMemory;
                if(ffRegReadUint64(hDirectxKey, L"DedicatedSystemMemory", &dedicatedSystemMemory, NULL) &&
                    ffRegReadUint64(hDirectxKey, L"SharedSystemMemory", &sharedSystemMemory, NULL))
                {
                    gpu->dedicated.total = dedicatedVideoMemory + dedicatedSystemMemory;
                    gpu->shared.total = sharedSystemMemory;
                }

                if (ffRegReadUint64(hDirectxKey, L"AdapterLuid", &adapterLuid, NULL))
                {
                    if (!gpu->deviceId) gpu->deviceId = adapterLuid;
                }

                uint32_t featureLevel = 0;
                if(ffRegReadUint(hDirectxKey, L"MaxD3D12FeatureLevel", &featureLevel, NULL) && featureLevel)
                    ffStrbufSetF(&gpu->platformApi, "Direct3D 12.%u", (featureLevel & 0x0F00) >> 8);
                else if(ffRegReadUint(hDirectxKey, L"MaxD3D11FeatureLevel", &featureLevel, NULL) && featureLevel)
                    ffStrbufSetF(&gpu->platformApi, "Direct3D 11.%u", (featureLevel & 0x0F00) >> 8);

                uint64_t driverVersion = 0;
                if(ffRegReadUint64(hDirectxKey, L"DriverVersion", &driverVersion, NULL) && driverVersion)
                {
                    ffStrbufSetF(&gpu->driver, "%u.%u.%u.%u",
                        (unsigned) (driverVersion >> 48) & 0xFFFF,
                        (unsigned) (driverVersion >> 32) & 0xFFFF,
                        (unsigned) (driverVersion >> 16) & 0xFFFF,
                        (unsigned) (driverVersion >> 0) & 0xFFFF
                    );
                }
            }
        }

        if (gpu->vendor.length == 0)
        {
            bufferLen = sizeof(buffer);
            if (SetupDiGetDeviceRegistryPropertyW(hdev, &did, SPDRP_DRIVER, NULL, (PBYTE) buffer, sizeof(buffer), &bufferLen) && bufferLen == (FF_GUID_STRLEN + strlen("\\0000") + 1) * 2)
            {
                wmemcpy(regDriverKey + regDriverKeyPrefixLength, buffer, FF_GUID_STRLEN + strlen("\\0000"));
                FF_HKEY_AUTO_DESTROY hRegDriverKey = NULL;
                if (ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regDriverKey, &hRegDriverKey, NULL))
                {
                    if (ffRegReadStrbuf(hRegDriverKey, L"ProviderName", &gpu->vendor, NULL))
                    {
                        if (ffStrbufContainS(&gpu->vendor, "Intel"))
                            ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
                        else if (ffStrbufContainS(&gpu->vendor, "NVIDIA"))
                            ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_NVIDIA);
                        else if (ffStrbufContainS(&gpu->vendor, "AMD") || ffStrbufContainS(&gpu->vendor, "ATI"))
                            ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD);
                    }
                }
            }
        }

        __typeof__(&ffDetectNvidiaGpuInfo) detectFn;
        const char* dllName;

        if (getDriverSpecificDetectionFn(gpu->vendor.chars, &detectFn, &dllName) && (options->temp || options->driverSpecific))
        {
            unsigned vendorId = 0, deviceId = 0, subSystemId = 0, revId = 0;
            if (SetupDiGetDeviceRegistryPropertyW(hdev, &did, SPDRP_HARDWAREID, NULL, (PBYTE) buffer, sizeof(buffer), NULL))
            {
                swscanf(buffer, L"PCI\\VEN_%x&DEV_%x&SUBSYS_%x&REV_%x", &vendorId, &deviceId, &subSystemId, &revId);
                ffStrbufSetStatic(&gpu->vendor, ffGetGPUVendorString(vendorId));
            }

            detectFn(
                &(FFGpuDriverCondition) {
                    .type = FF_GPU_DRIVER_CONDITION_TYPE_DEVICE_ID
                            | (adapterLuid > 0 ? FF_GPU_DRIVER_CONDITION_TYPE_LUID : 0)
                            | (pciAddr > 0 ? FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID : 0),
                    .pciDeviceId = {
                        .deviceId = deviceId,
                        .vendorId = vendorId,
                        .subSystemId = subSystemId,
                        .revId = revId,
                    },
                    .pciBusId = {
                        .domain = 0,
                        .bus = pciBus,
                        .device = pciDev,
                        .func = pciFunc,
                    },
                    .luid = adapterLuid,
                },
                (FFGpuDriverResult){
                    .index = &gpu->index,
                    .temp = options->temp ? &gpu->temperature : NULL,
                    .memory = options->driverSpecific ? &gpu->dedicated : NULL,
                    .coreCount = options->driverSpecific ? (uint32_t*) &gpu->coreCount : NULL,
                    .coreUsage = options->driverSpecific ? &gpu->coreUsage : NULL,
                    .type = &gpu->type,
                    .name = &gpu->name,
                    .frequency = options->driverSpecific ? &gpu->frequency : NULL,
                },
                dllName
            );
        }

        if (!gpu->name.length)
        {
            if (SetupDiGetDeviceRegistryPropertyW(hdev, &did, SPDRP_DEVICEDESC, NULL, (PBYTE) buffer, sizeof(buffer), NULL))
                ffStrbufSetWS(&gpu->name, buffer);
        }
    }

    return NULL;
}
