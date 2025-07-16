#include "gpu_driver_specific.h"

#include "common/library.h"
#include "nvml.h"

struct FFNvmlData {
    FF_LIBRARY_SYMBOL(nvmlDeviceGetCount_v2)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetHandleByIndex_v2)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetHandleByPciBusId_v2)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetPciInfo_v3)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetTemperature)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetMemoryInfo_v2)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetMemoryInfo)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetNumGpuCores)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetMaxClockInfo)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetUtilizationRates)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetBrand)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetIndex)
    FF_LIBRARY_SYMBOL(nvmlDeviceGetName)

    bool inited;
} nvmlData;

#if defined(_WIN32) && !defined(FF_DISABLE_DLOPEN)

#include "nvapi.h"

struct FFNvapiData {
    FF_LIBRARY_SYMBOL(nvapi_Unload)
    FF_LIBRARY_SYMBOL(nvapi_EnumPhysicalGPUs)
    FF_LIBRARY_SYMBOL(nvapi_GPU_GetRamType)
    FF_LIBRARY_SYMBOL(nvapi_GPU_GetGPUType)

    bool inited;
} nvapiData;

static const char* detectMoreByNvapi(FFGpuDriverResult* result)
{
    if (!nvapiData.inited)
    {
        nvapiData.inited = true;

        FF_LIBRARY_LOAD(libnvapi, "dlopen nvapi failed",
            #ifdef _WIN64
            "nvapi64.dll"
            #else
            "nvapi.dll"
            #endif
        , 1);
        FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libnvapi, nvapi_QueryInterface)
        #define FF_NVAPI_INTERFACE(iName, iOffset) \
            __typeof__(&iName) ff ## iName = ffnvapi_QueryInterface(iOffset); \
            if (ff ## iName == NULL) return "nvapi_QueryInterface " #iName " failed";

        FF_NVAPI_INTERFACE(nvapi_Initialize, NVAPI_INTERFACE_OFFSET_INITIALIZE)
        FF_NVAPI_INTERFACE(nvapi_Unload, NVAPI_INTERFACE_OFFSET_UNLOAD)
        FF_NVAPI_INTERFACE(nvapi_EnumPhysicalGPUs, NVAPI_INTERFACE_OFFSET_ENUM_PHYSICAL_GPUS)
        FF_NVAPI_INTERFACE(nvapi_GPU_GetRamType, NVAPI_INTERFACE_OFFSET_GPU_GET_RAM_TYPE)
        FF_NVAPI_INTERFACE(nvapi_GPU_GetGPUType, NVAPI_INTERFACE_OFFSET_GPU_GET_GPU_TYPE)
        #undef FF_NVAPI_INTERFACE

        if (ffnvapi_Initialize() < 0)
            return "NvAPI_Initialize() failed";

        nvapiData.ffnvapi_EnumPhysicalGPUs = ffnvapi_EnumPhysicalGPUs;
        nvapiData.ffnvapi_GPU_GetRamType = ffnvapi_GPU_GetRamType;
        nvapiData.ffnvapi_GPU_GetGPUType = ffnvapi_GPU_GetGPUType;
        nvapiData.ffnvapi_Unload = ffnvapi_Unload;

        atexit((void*) ffnvapi_Unload);
        libnvapi = NULL; // don't close nvapi
    }

    if (nvapiData.ffnvapi_EnumPhysicalGPUs == NULL)
        return "loading nvapi library failed";

    NvPhysicalGpuHandle handles[32];
    int gpuCount = 0;

    if (nvapiData.ffnvapi_EnumPhysicalGPUs(handles, &gpuCount) < 0)
        return "NvAPI_EnumPhysicalGPUs() failed";

    uint32_t gpuIndex = *result->index;

    if (gpuIndex >= (uint32_t) gpuCount)
        return "GPU index out of range";

    // Not very sure. Need to check in multi-GPU system
    NvPhysicalGpuHandle gpuHandle = handles[gpuIndex];

    NvApiGPUMemoryType memType;
    if (result->memoryType && nvapiData.ffnvapi_GPU_GetRamType(gpuHandle, &memType) == 0)
    {
        switch (memType)
        {
            #define FF_NVAPI_MEMORY_TYPE(type) \
                case NVAPI_GPU_MEMORY_TYPE_##type: \
                    ffStrbufSetStatic(result->memoryType, #type); \
                    break;
            FF_NVAPI_MEMORY_TYPE(UNKNOWN)
            FF_NVAPI_MEMORY_TYPE(SDRAM)
            FF_NVAPI_MEMORY_TYPE(DDR1)
            FF_NVAPI_MEMORY_TYPE(DDR2)
            FF_NVAPI_MEMORY_TYPE(GDDR2)
            FF_NVAPI_MEMORY_TYPE(GDDR3)
            FF_NVAPI_MEMORY_TYPE(GDDR4)
            FF_NVAPI_MEMORY_TYPE(DDR3)
            FF_NVAPI_MEMORY_TYPE(GDDR5)
            FF_NVAPI_MEMORY_TYPE(LPDDR2)
            FF_NVAPI_MEMORY_TYPE(GDDR5X)
            FF_NVAPI_MEMORY_TYPE(LPDDR3)
            FF_NVAPI_MEMORY_TYPE(LPDDR4)
            FF_NVAPI_MEMORY_TYPE(LPDDR5)
            FF_NVAPI_MEMORY_TYPE(GDDR6)
            FF_NVAPI_MEMORY_TYPE(GDDR6X)
            FF_NVAPI_MEMORY_TYPE(GDDR7)
            #undef FF_NVAPI_MEMORY_TYPE
            default:
                ffStrbufSetF(result->memoryType, "Unknown (%d)", memType);
                break;
        }
    }

    NvApiGPUType gpuType;
    if (result->type && nvapiData.ffnvapi_GPU_GetGPUType(gpuHandle, &gpuType) == 0)
    {
        switch (gpuType)
        {
            case NV_SYSTEM_TYPE_IGPU:
                *result->type = FF_GPU_TYPE_INTEGRATED;
                break;
            case NV_SYSTEM_TYPE_DGPU:
                *result->type = FF_GPU_TYPE_DISCRETE;
                break;
            default:
                *result->type = FF_GPU_TYPE_UNKNOWN;
                break;
        }
    }

    return NULL;
}

#endif

const char* ffDetectNvidiaGpuInfo(const FFGpuDriverCondition* cond, FFGpuDriverResult result, const char* soName)
{
#ifndef FF_DISABLE_DLOPEN

    if (!nvmlData.inited)
    {
        nvmlData.inited = true;
        FF_LIBRARY_LOAD(libnvml, "dlopen nvml failed", soName , 1);
        FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libnvml, nvmlInit_v2)
        FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libnvml, nvmlShutdown)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetCount_v2)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetHandleByIndex_v2)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetHandleByPciBusId_v2)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetPciInfo_v3)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetTemperature)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetMemoryInfo_v2)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetMemoryInfo)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetNumGpuCores)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetMaxClockInfo)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetUtilizationRates)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetBrand)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetIndex)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libnvml, nvmlData, nvmlDeviceGetName)

        if (ffnvmlInit_v2() != NVML_SUCCESS)
        {
            nvmlData.ffnvmlDeviceGetNumGpuCores = NULL;
            return "nvmlInit_v2() failed";
        }
        atexit((void*) ffnvmlShutdown);
        libnvml = NULL; // don't close nvml
    }

    if (nvmlData.ffnvmlDeviceGetNumGpuCores == NULL)
        return "loading nvml library failed";

    nvmlDevice_t device = NULL;
    if (cond->type & FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID)
    {
        char pciBusIdStr[32];
        snprintf(pciBusIdStr, ARRAY_SIZE(pciBusIdStr), "%04x:%02x:%02x.%d", cond->pciBusId.domain, cond->pciBusId.bus, cond->pciBusId.device, cond->pciBusId.func);

        nvmlReturn_t ret = nvmlData.ffnvmlDeviceGetHandleByPciBusId_v2(pciBusIdStr, &device);
        if (ret != NVML_SUCCESS)
            return "nvmlDeviceGetHandleByPciBusId_v2() failed";
    }
    else if (cond->type & FF_GPU_DRIVER_CONDITION_TYPE_DEVICE_ID)
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

            if (pciInfo.pciDeviceId != ((cond->pciDeviceId.deviceId << 16u) | cond->pciDeviceId.vendorId) ||
                pciInfo.pciSubSystemId != cond->pciDeviceId.subSystemId)
                continue;

            break;
        }
    }

    if (!device) return "Device not found";

    if (result.type)
    {
        nvmlBrandType_t brand;
        if (nvmlData.ffnvmlDeviceGetBrand(device, &brand) == NVML_SUCCESS)
        {
            switch (brand)
            {
                case NVML_BRAND_NVIDIA_RTX:
                case NVML_BRAND_QUADRO_RTX:
                case NVML_BRAND_GEFORCE:
                case NVML_BRAND_TITAN:
                case NVML_BRAND_TESLA:
                case NVML_BRAND_QUADRO:
                    *result.type = FF_GPU_TYPE_DISCRETE;
                    break;
                default:
                    break;
            }
        }
    }

    if (result.index)
    {
        unsigned int value;
        if (nvmlData.ffnvmlDeviceGetIndex(device, &value) == NVML_SUCCESS)
        {
            *result.index = value;
            #ifdef _WIN32
            // Don't bother loading nvapi for GPU type detection only
            if (result.memoryType)
                detectMoreByNvapi(&result);
            #endif
        }
    }

    if (result.temp)
    {
        uint32_t value;
        if (nvmlData.ffnvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &value) == NVML_SUCCESS)
            *result.temp = value;
    }

    if (result.memory)
    {
        nvmlMemory_v2_t memory = { .version = nvmlMemory_v2 };
        if (nvmlData.ffnvmlDeviceGetMemoryInfo_v2(device, &memory) == NVML_SUCCESS)
        {
            result.memory->total = memory.used + memory.free;
            result.memory->used = memory.used;
        }
        else
        {
            nvmlMemory_t memory_v1;
            if (nvmlData.ffnvmlDeviceGetMemoryInfo(device, &memory_v1) == NVML_SUCCESS)
            {
                result.memory->total = memory_v1.total;
                result.memory->used = memory_v1.used;
            }
        }
    }

    if (result.coreCount)
        nvmlData.ffnvmlDeviceGetNumGpuCores(device, result.coreCount);

    if (result.frequency)
        nvmlData.ffnvmlDeviceGetMaxClockInfo(device, NVML_CLOCK_GRAPHICS, result.frequency);

    if (result.coreUsage)
    {
        nvmlUtilization_t utilization;
        if (nvmlData.ffnvmlDeviceGetUtilizationRates(device, &utilization) == NVML_SUCCESS)
            *result.coreUsage = utilization.gpu;
    }

    if (result.name)
    {
        char name[NVML_DEVICE_NAME_V2_BUFFER_SIZE];
        if (nvmlData.ffnvmlDeviceGetName(device, name, ARRAY_SIZE(name)) == NVML_SUCCESS)
            ffStrbufSetS(result.name, name);
    }

    return NULL;

#else

    FF_UNUSED(cond, result, soName);
    return "dlopen is disabled";

#endif
}
