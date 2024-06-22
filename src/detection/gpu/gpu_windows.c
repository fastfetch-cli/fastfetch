#include "gpu.h"
#include "detection/gpu/gpu_driver_specific.h"
#include "util/windows/unicode.h"
#include "util/windows/registry.h"

#include <inttypes.h>

static int isGpuNameEqual(const FFGPUResult* gpu, const FFstrbuf* name)
{
    return ffStrbufEqual(&gpu->name, name);
}

static inline bool getDriverSpecificDetectionFn(const char* vendor, __typeof__(&ffDetectNvidiaGpuInfo)* pDetectFn, const char** pDllName)
{
    if (vendor == FF_GPU_VENDOR_NAME_NVIDIA)
    {
        *pDetectFn = ffDetectNvidiaGpuInfo;
        *pDllName = "nvml.dll";
    }
    else if (vendor == FF_GPU_VENDOR_NAME_INTEL)
    {
        *pDetectFn = ffDetectIntelGpuInfo;
        #ifdef _WIN64
            *pDllName = "ControlLib.dll";
        #else
            *pDllName = "ControlLib32.dll";
        #endif
    }
    else if (vendor == FF_GPU_VENDOR_NAME_AMD)
    {
        *pDetectFn = ffDetectAmdGpuInfo;
        #ifdef _WIN64
            *pDllName = "amd_ags_x64.dll";
        #else
            *pDllName = "amd_ags_x86.dll";
        #endif
    }
    else
    {
        *pDetectFn = NULL;
        *pDllName = NULL;
        return false;
    }

    return true;
}

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    DISPLAY_DEVICEW displayDevice = { .cb = sizeof(displayDevice) };
    wchar_t regDirectxKey[MAX_PATH] = L"SOFTWARE\\Microsoft\\DirectX\\{";
    const uint32_t regDirectxKeyPrefixLength = (uint32_t) wcslen(regDirectxKey);
    wchar_t regControlVideoKey[MAX_PATH] = L"SYSTEM\\CurrentControlSet\\Control\\Video\\{";
    const uint32_t regControlVideoKeyPrefixLength = (uint32_t) wcslen(regControlVideoKey);
    const uint32_t deviceKeyPrefixLength = strlen("\\Registry\\Machine\\") + regControlVideoKeyPrefixLength;

    for (DWORD i = 0; EnumDisplayDevicesW(NULL, i, &displayDevice, 0); ++i)
    {
        if (displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) continue;

        const uint32_t deviceKeyLength = (uint32_t) wcslen(displayDevice.DeviceKey);
        if (__builtin_expect(deviceKeyLength == 100, true))
        {
            if (wmemcmp(&displayDevice.DeviceKey[deviceKeyLength - 4], L"0000", 4) != 0) continue;
        }
        else
        {
            // DeviceKey can be empty. See #484
            FF_STRBUF_AUTO_DESTROY gpuName = ffStrbufCreateWS(displayDevice.DeviceString);
            if (ffListContains(gpus, &gpuName, (void*) isGpuNameEqual)) continue;
        }

        // See: https://download.nvidia.com/XFree86/Linux-x86_64/545.23.06/README/supportedchips.html
        // displayDevice.DeviceID = MatchingDeviceId "PCI\\VEN_10DE&DEV_2782&SUBSYS_513417AA&REV_A1"
        unsigned vendorId = 0, deviceId = 0, subSystemId = 0, revId = 0;
        swscanf(displayDevice.DeviceID, L"PCI\\VEN_%x&DEV_%x&SUBSYS_%x&REV_%x", &vendorId, &deviceId, &subSystemId, &revId);

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInitStatic(&gpu->vendor, ffGetGPUVendorString(vendorId));
        ffStrbufInitWS(&gpu->name, displayDevice.DeviceString);
        ffStrbufInit(&gpu->driver);
        ffStrbufInitStatic(&gpu->platformApi, "Direct3D");
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = 0;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        if (deviceKeyLength == 100 && displayDevice.DeviceKey[deviceKeyPrefixLength - 1] == '{')
        {
            wmemcpy(regControlVideoKey + regControlVideoKeyPrefixLength, displayDevice.DeviceKey + deviceKeyPrefixLength, strlen("00000000-0000-0000-0000-000000000000}\\0000"));
            FF_HKEY_AUTO_DESTROY hKey = NULL;
            if (!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regControlVideoKey, &hKey, NULL)) continue;

            ffRegReadStrbuf(hKey, L"DriverVersion", &gpu->driver, NULL);

            wmemcpy(regDirectxKey + regDirectxKeyPrefixLength, displayDevice.DeviceKey + deviceKeyPrefixLength, strlen("00000000-0000-0000-0000-000000000000}"));
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
        }

        __typeof__(&ffDetectNvidiaGpuInfo) detectFn;
        const char* dllName;

        if (getDriverSpecificDetectionFn(gpu->vendor.chars, &detectFn, &dllName) && (options->temp || options->driverSpecific))
        {
            if (vendorId && deviceId && subSystemId && revId)
            {
                detectFn(
                    &(FFGpuDriverCondition) {
                        .type = FF_GPU_DRIVER_CONDITION_TYPE_DEVICE_ID | FF_GPU_DRIVER_CONDITION_TYPE_LUID,
                        .pciDeviceId = {
                            .deviceId = deviceId,
                            .vendorId = vendorId,
                            .subSystemId = subSystemId,
                            .revId = revId,
                        },
                        .luid = gpu->deviceId,
                    },
                    (FFGpuDriverResult) {
                        .temp = options->temp ? &gpu->temperature : NULL,
                        .memory = options->driverSpecific ? &gpu->dedicated : NULL,
                        .coreCount = options->driverSpecific ? (uint32_t*) &gpu->coreCount : NULL,
                        .type = &gpu->type,
                        .frequency = &gpu->frequency,
                    },
                    dllName
                );
            }
        }
    }

    return NULL;
}
