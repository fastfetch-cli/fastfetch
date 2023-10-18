#include "gpu.h"
#include "util/windows/unicode.h"
#include "util/windows/registry.h"

#include <inttypes.h>

static int isGpuNameEqual(const FFGPUResult* gpu, const FFstrbuf* name)
{
    return ffStrbufEqual(&gpu->name, name);
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

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInit(&gpu->vendor);
        ffStrbufInitWS(&gpu->name, displayDevice.DeviceString);
        ffStrbufInit(&gpu->driver);
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;

        if (deviceKeyLength == 100 && displayDevice.DeviceKey[deviceKeyPrefixLength - 1] == '{')
        {
            wmemcpy(regControlVideoKey + regControlVideoKeyPrefixLength, displayDevice.DeviceKey + deviceKeyPrefixLength, strlen("00000000-0000-0000-0000-000000000000}\\0000"));
            FF_HKEY_AUTO_DESTROY hKey = NULL;
            if (!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regControlVideoKey, &hKey, NULL)) continue;

            ffRegReadStrbuf(hKey, L"DriverVersion", &gpu->driver, NULL);
            ffRegReadStrbuf(hKey, L"ProviderName", &gpu->vendor, NULL);

            if(ffStrbufContainS(&gpu->vendor, "AMD") || ffStrbufContainS(&gpu->vendor, "ATI"))
                ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD);
            else if(ffStrbufContainS(&gpu->vendor, "Intel"))
                ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
            else if(ffStrbufContainS(&gpu->vendor, "NVIDIA"))
                ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_NVIDIA);

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
            }
            else if (!ffRegReadUint64(hKey, L"HardwareInformation.qwMemorySize", &gpu->dedicated.total, NULL))
            {
                uint32_t vmem = 0;
                if (ffRegReadUint(hKey, L"HardwareInformation.MemorySize", &vmem, NULL))
                    gpu->dedicated.total = vmem;
                gpu->type = gpu->dedicated.total > 1024 * 1024 * 1024 ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;
            }
        }
    }

    return NULL;
}
