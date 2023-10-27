#include "temps_nvidia.h"

#include "3rdparty/nvml/nvml.h"
#include "common/library.h"

struct FFNvmlData {
    FF_LIBRARY_SYMBOL(nvmlDeviceGetCount_v2)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetHandleByIndex_v2)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetHandleByPciBusId_v2)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetPciInfo_v3)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetTemperature)

    bool inited;
} nvmlData;

// Use pciBusId if not NULL; use pciDeviceId and pciSubSystemId otherwise
// pciBusId = "domain:bus:device.function"
// pciDeviceId = (deviceId << 16) | vendorId
const char* ffDetectNvidiaGpuTemp(double* temp, const char* pciBusId, uint32_t pciDeviceId, uint32_t pciSubSystemId)
{
    if (!nvmlData.inited)
    {
        nvmlData.inited = true;
        FF_LIBRARY_LOAD(libnvml, NULL, "dlopen nvml failed",
#ifdef _WIN32
            "nvml"
#else
            "libnvidia-ml"
#endif
        FF_LIBRARY_EXTENSION, -1
        );
        FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libnvml, nvmlInit_v2)
        FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libnvml, nvmlShutdown)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetCount_v2)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetHandleByIndex_v2)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetHandleByPciBusId_v2)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetPciInfo_v3)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetTemperature)

        if (ffnvmlInit_v2() != NVML_SUCCESS)
        {
            nvmlData.ffnvmlDeviceGetTemperature = NULL;
            return "nvmlInit_v2() failed";
        }
        atexit((void*) ffnvmlShutdown);
        libnvml = NULL; // don't close nvml
    }

    if (nvmlData.ffnvmlDeviceGetTemperature == NULL)
        return "loading nvml library failed";

    nvmlDevice_t device = NULL;
    if (pciBusId)
    {
        nvmlReturn_t ret = nvmlData.ffnvmlDeviceGetHandleByPciBusId_v2(pciBusId, &device);
        if (ret != NVML_SUCCESS)
            return "nvmlDeviceGetHandleByPciBusId_v2() failed";
    }
    else
    {
        uint32_t count;
        if (nvmlData.ffnvmlDeviceGetCount_v2(&count) != NVML_SUCCESS)
            return "nvmlDeviceGetCount_v2() failed";

        for (uint32_t i = 0; i < count; i++, device = NULL)
        {
            if (nvmlData.ffnvmlDeviceGetHandleByIndex_v2(i, &device) != NVML_SUCCESS)
                continue;

            nvmlPciInfo_t pciInfo;
            if (nvmlData.ffnvmlDeviceGetPciInfo_v3(device, &pciInfo) != NVML_SUCCESS)
                continue;

            if (pciInfo.pciDeviceId != pciDeviceId || pciInfo.pciSubSystemId != pciSubSystemId)
                continue;

            break;
        }
        if (!device) return "Device not found";
    }

    uint32_t value;
    if (nvmlData.ffnvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &value) != NVML_SUCCESS)
        return "nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &value) failed";

    *temp = value;
    return NULL;
}
