#include "gpu.h"
#include "util/windows/unicode.h"
#include "util/windows/registry.h"

#include <inttypes.h>

const char* ffDetectGPUImpl(FFlist* gpus, FF_MAYBE_UNUSED const FFinstance* instance)
{
    DISPLAY_DEVICEW displayDevice = {.cb = sizeof(displayDevice) };
    wchar_t regKey[MAX_PATH] = L"SYSTEM\\CurrentControlSet\\Control\\Video\\{";
    const uint32_t regKeyPrefixLength = strlen("SYSTEM\\CurrentControlSet\\Control\\Video\\{");
    const uint32_t deviceKeyPrefixLength = strlen("\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Video\\{");
    for (DWORD i = 0; EnumDisplayDevicesW(NULL, i, &displayDevice, 0); ++i)
    {
        if (displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) continue;

        assert(wcslen(displayDevice.DeviceKey) == 100);
        assert(wmemcmp(displayDevice.DeviceKey, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Video\\{", deviceKeyPrefixLength) == 0);
        assert(displayDevice.DeviceKey[95] == L'\\');

        if (wmemcmp(&displayDevice.DeviceKey[96], L"0000", 4) != 0) continue;

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInit(&gpu->vendor);
        ffStrbufInit(&gpu->name);
        ffStrbufInit(&gpu->driver);
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->id = 0;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;

        ffStrbufSetWS(&gpu->name, displayDevice.DeviceString);

        wmemcpy(regKey + regKeyPrefixLength, displayDevice.DeviceKey + deviceKeyPrefixLength, 100 - regKeyPrefixLength + 1);
        FF_HKEY_AUTO_DESTROY hKey = NULL;
        if (!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regKey, &hKey, NULL)) continue;

        ffRegReadStrbuf(hKey, L"DriverVersion", &gpu->driver, NULL);
        ffRegReadStrbuf(hKey, L"ProviderName", &gpu->vendor, NULL);

        if(ffStrbufContainS(&gpu->vendor, "AMD") || ffStrbufContainS(&gpu->vendor, "ATI"))
            ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD);
        else if(ffStrbufContainS(&gpu->vendor, "Intel"))
            ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
        else if(ffStrbufContainS(&gpu->vendor, "NVIDIA"))
            ffStrbufSetS(&gpu->vendor, FF_GPU_VENDOR_NAME_NVIDIA);

        if (!ffRegReadUint64(hKey, L"HardwareInformation.qwMemorySize", &gpu->dedicated.total, NULL))
        {
            uint32_t vmem = 0;
            if (ffRegReadUint(hKey, L"HardwareInformation.MemorySize", &vmem, NULL))
                gpu->dedicated.total = vmem;
        }
        gpu->type = gpu->dedicated.total > 1024 * 1024 * 1024 ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;
    }

    return NULL;
}
