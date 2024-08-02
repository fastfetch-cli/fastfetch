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
#define FF_GUID_STRLEN (sizeof(FF_EMPTY_GUID_STR) / sizeof(wchar_t) - 1)

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    HDEVINFO hdev __attribute__((__cleanup__(wrapSetupDiDestroyDeviceInfoList))) =
        SetupDiGetClassDevsW(&GUID_DEVCLASS_DISPLAY, NULL, NULL, DIGCF_PRESENT);

    if(hdev == INVALID_HANDLE_VALUE)
        return "SetupDiGetClassDevsW(&GUID_DEVCLASS_DISPLAY) failed";

    wchar_t regDirectxKey[] = L"SOFTWARE\\Microsoft\\DirectX\\" FF_EMPTY_GUID_STR;
    const uint32_t regDirectxKeyPrefixLength = (uint32_t) strlen("SOFTWARE\\Microsoft\\DirectX\\");
    wchar_t regControlVideoKey[] = L"SYSTEM\\CurrentControlSet\\Control\\Video\\" FF_EMPTY_GUID_STR L"\\0000";
    const uint32_t regControlVideoKeyPrefixLength = (uint32_t) strlen("SYSTEM\\CurrentControlSet\\Control\\Video\\");

    SP_DEVINFO_DATA did = { .cbSize = sizeof(did) };
    for (DWORD idev = 0; SetupDiEnumDeviceInfo(hdev, idev, &did); ++idev)
    {
        wchar_t hardwareId[256];
        if (!SetupDiGetDeviceRegistryPropertyW(hdev, &did, SPDRP_HARDWAREID, NULL, (PBYTE) hardwareId, sizeof(hardwareId), NULL))
            continue;

        wchar_t videoId[39];
        {
            FF_HKEY_AUTO_DESTROY hKey = SetupDiOpenDevRegKey(hdev, &did, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
            if (!hKey) continue;
            DWORD videoIdLength = sizeof(videoId);
            if (RegGetValueW(hKey, NULL, L"VideoID", RRF_RT_REG_SZ, NULL, videoId, &videoIdLength) != ERROR_SUCCESS || videoIdLength != (FF_GUID_STRLEN + 1) * sizeof(wchar_t))
                continue;
        }

        wmemcpy(regControlVideoKey + regControlVideoKeyPrefixLength, videoId, FF_GUID_STRLEN);
        FF_HKEY_AUTO_DESTROY hKey = NULL;
        if (!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regControlVideoKey, &hKey, NULL)) continue;

        unsigned vendorId = 0, deviceId = 0, subSystemId = 0, revId = 0;
        swscanf(hardwareId, L"PCI\\VEN_%x&DEV_%x&SUBSYS_%x&REV_%x", &vendorId, &deviceId, &subSystemId, &revId);

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInit(&gpu->vendor);
        ffStrbufInit(&gpu->name);
        ffStrbufInit(&gpu->driver);
        ffStrbufInitStatic(&gpu->platformApi, "Direct3D");
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = 0;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        if (!ffRegReadStrbuf(hKey, L"DriverDesc", &gpu->name, NULL)) // faster, for some reason
        {
            wchar_t name[256];
            if (SetupDiGetDeviceRegistryPropertyW(hdev, &did, SPDRP_DEVICEDESC, NULL, (PBYTE) name, sizeof(name), NULL))
                ffStrbufSetWS(&gpu->name, name);
        }
        ffRegReadStrbuf(hKey, L"DriverVersion", &gpu->driver, NULL);

        wmemcpy(regDirectxKey + regDirectxKeyPrefixLength, videoId, FF_GUID_STRLEN);
        FF_HKEY_AUTO_DESTROY hDirectxKey = NULL;
        if (ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regDirectxKey, &hDirectxKey, NULL))
        {
            uint64_t dedicatedVideoMemory = 0;
            if(ffRegReadUint64(hDirectxKey, L"DedicatedVideoMemory", &dedicatedVideoMemory, NULL))
                gpu->type = dedicatedVideoMemory >= 1024 * 1024 * 1024 ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;

            uint64_t dedicatedSystemMemory, sharedSystemMemory;
            if(ffRegReadUint64(hDirectxKey, L"DedicatedSystemMemory", &dedicatedSystemMemory, NULL) &&
                ffRegReadUint64(hDirectxKey, L"SharedSystemMemory", &sharedSystemMemory, NULL))
            {
                gpu->dedicated.total = dedicatedVideoMemory + dedicatedSystemMemory;
                gpu->shared.total = sharedSystemMemory;
            }

            ffRegReadUint64(hDirectxKey, L"AdapterLuid", &gpu->deviceId, NULL);

            uint32_t featureLevel = 0;
            if(ffRegReadUint(hDirectxKey, L"MaxD3D12FeatureLevel", &featureLevel, NULL) && featureLevel)
                ffStrbufSetF(&gpu->platformApi, "Direct3D 12.%u", (featureLevel & 0x0F00) >> 8);
            else if(ffRegReadUint(hDirectxKey, L"MaxD3D11FeatureLevel", &featureLevel, NULL) && featureLevel)
                ffStrbufSetF(&gpu->platformApi, "Direct3D 11.%u", (featureLevel & 0x0F00) >> 8);
        }
        else if (!ffRegReadUint64(hKey, L"HardwareInformation.qwMemorySize", &gpu->dedicated.total, NULL))
        {
            uint32_t vmem = 0;
            if (ffRegReadUint(hKey, L"HardwareInformation.MemorySize", &vmem, NULL))
                gpu->dedicated.total = vmem;
            gpu->type = gpu->dedicated.total > 1024 * 1024 * 1024 ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;
        }

        if (gpu->vendor.length == 0)
        {
            ffRegReadStrbuf(hKey, L"ProviderName", &gpu->vendor, NULL);
            if (ffStrbufContainS(&gpu->vendor, "Intel"))
                ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
            else if (ffStrbufContainS(&gpu->vendor, "NVIDIA"))
                ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_NVIDIA);
            else if (ffStrbufContainS(&gpu->vendor, "AMD") || ffStrbufContainS(&gpu->vendor, "ATI"))
                ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD);
        }

        __typeof__(&ffDetectNvidiaGpuInfo) detectFn;
        const char* dllName;

        if (getDriverSpecificDetectionFn(gpu->vendor.chars, &detectFn, &dllName) && (options->temp || options->driverSpecific))
        {
            uint32_t pciBus, pciAddr;
            if (SetupDiGetDeviceRegistryPropertyW(hdev, &did, SPDRP_BUSNUMBER, NULL, (PBYTE) &pciBus, sizeof(pciBus), NULL) &&
                SetupDiGetDeviceRegistryPropertyW(hdev, &did, SPDRP_ADDRESS, NULL, (PBYTE) &pciAddr, sizeof(pciAddr), NULL))
            {
                uint32_t pciDev = (pciAddr >> 16) & 0xFFFF;
                uint32_t pciFunc = pciAddr & 0xFFFF;

                detectFn(
                    &(FFGpuDriverCondition) {
                        .type = FF_GPU_DRIVER_CONDITION_TYPE_DEVICE_ID
                              | (gpu->deviceId > 0 ? FF_GPU_DRIVER_CONDITION_TYPE_LUID : 0)
                              | FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID,
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
                        .luid = gpu->deviceId,
                    },
                    (FFGpuDriverResult) {
                        .temp = options->temp ? &gpu->temperature : NULL,
                        .memory = options->driverSpecific ? &gpu->dedicated : NULL,
                        .coreCount = options->driverSpecific ? (uint32_t*) &gpu->coreCount : NULL,
                        .coreUsage = options->driverSpecific ? &gpu->coreUsage : NULL,
                        .type = &gpu->type,
                        .frequency = options->driverSpecific ? &gpu->frequency : NULL,
                    },
                    dllName
                );
            }
        }
    }

    return NULL;
}
