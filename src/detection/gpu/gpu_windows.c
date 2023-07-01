#include "gpu.h"
#include "util/windows/unicode.h"
#include "util/windows/registry.h"

#include <inttypes.h>

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    fputs("ffDetectGPUImpl: start\n", stderr);
    DISPLAY_DEVICEW displayDevice = { .cb = sizeof(displayDevice) };
    wchar_t regKey[MAX_PATH] = L"SYSTEM\\CurrentControlSet\\Control\\Video\\{";
    const uint32_t regKeyPrefixLength = (uint32_t) wcslen(regKey);
    const uint32_t deviceKeyPrefixLength = strlen("\\Registry\\Machine\\") + regKeyPrefixLength;

    for (DWORD i = 0; EnumDisplayDevicesW(NULL, i, &displayDevice, 0); ++i)
    {
        if (displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) continue;

        fwprintf(stderr, L"ffDetectGPUImpl EnumDisplayDevicesW %ls\n", displayDevice.DeviceKey);
        const uint32_t deviceKeyLength = (uint32_t) wcslen(displayDevice.DeviceKey);
        fprintf(stderr, "ffDetectGPUImpl deviceKeyLength %d\n", (int) deviceKeyLength);
        if (deviceKeyLength != 100 || wmemcmp(&displayDevice.DeviceKey[deviceKeyLength - 4], L"0000", 4) != 0) continue;

        fputs("ffDetectGPUImpl ffListAdd\n", stderr);
        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInit(&gpu->vendor);
        ffStrbufInitWS(&gpu->name, displayDevice.DeviceString);
        ffStrbufInit(&gpu->driver);
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;

        if (displayDevice.DeviceKey[deviceKeyPrefixLength - 1] == '{')
        {
            fputs("ffDetectGPUImpl displayDevice.DeviceKey[deviceKeyPrefixLength - 1]\n", stderr);
            wmemcpy(regKey + regKeyPrefixLength, displayDevice.DeviceKey + deviceKeyPrefixLength, 100 - regKeyPrefixLength + 1);
            FF_HKEY_AUTO_DESTROY hKey = NULL;
            fputs("ffDetectGPUImpl ffRegOpenKeyForRead start\n", stderr);
            if (!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regKey, &hKey, NULL)) continue;
            fputs("ffDetectGPUImpl ffRegOpenKeyForRead end\n", stderr);

            ffRegReadStrbuf(hKey, L"DriverVersion", &gpu->driver, NULL);
            fputs("ffDetectGPUImpl DriverVersion end\n", stderr);
            ffRegReadStrbuf(hKey, L"ProviderName", &gpu->vendor, NULL);
            fputs("ffDetectGPUImpl ProviderName end\n", stderr);

            if(ffStrbufContainS(&gpu->vendor, "AMD") || ffStrbufContainS(&gpu->vendor, "ATI"))
                ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD);
            else if(ffStrbufContainS(&gpu->vendor, "Intel"))
                ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
            else if(ffStrbufContainS(&gpu->vendor, "NVIDIA"))
                ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_NVIDIA);

            fputs("ffDetectGPUImpl HardwareInformation.qwMemorySize start\n", stderr);
            if (!ffRegReadUint64(hKey, L"HardwareInformation.qwMemorySize", &gpu->dedicated.total, NULL))
            {
                fputs("ffDetectGPUImpl HardwareInformation.MemorySize start\n", stderr);
                uint32_t vmem = 0;
                if (ffRegReadUint(hKey, L"HardwareInformation.MemorySize", &vmem, NULL))
                    gpu->dedicated.total = vmem;
            }
            gpu->type = gpu->dedicated.total > 1024 * 1024 * 1024 ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;
        }
    }
    fputs("ffDetectGPUImpl end\n", stderr);

    return NULL;
}
