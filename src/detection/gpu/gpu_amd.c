#include "gpu_driver_specific.h"

// Everything detected in this file is static.
// The real time monitoring requires ADLX, whose interface is much more complicated than AGS
// Whoever has AMD graphic cards interested in this may contribute
// * ADLX (AMD Device Library eXtra): https://github.com/GPUOpen-LibrariesAndSDKs/ADLX

#include "3rdparty/ags/amd_ags.h"
#include "common/library.h"
#include "util/mallocHelper.h"

const char* ffDetectAmdGpuInfo(const FFGpuDriverCondition* cond, FFGpuDriverResult result, const char* soName)
{
    static bool inited = false;
    static AGSGPUInfo gpuInfo;

    if (!inited)
    {
        inited = true;
        FF_LIBRARY_LOAD(libags, "dlopen amd_ags failed", soName , 1);
        FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libags, agsInitialize)

        struct AGSContext* apiHandle;
        if (ffagsInitialize(AGS_CURRENT_VERSION, NULL, &apiHandle, &gpuInfo) != AGS_SUCCESS)
            return "loading ags library failed";

        // agsDeInitialize will free pointers allocated in gpuInfo. Just leak them.
    }

    if (gpuInfo.numDevices == 0)
        return "loading ags library failed or no AMD gpus found";

    AGSDeviceInfo* device = NULL;

    for (int iDev = 0; iDev < gpuInfo.numDevices; iDev++)
    {
        if (cond->type & FF_GPU_DRIVER_CONDITION_TYPE_DEVICE_ID)
        {
            if (
                cond->pciDeviceId.deviceId == (uint32_t) gpuInfo.devices[iDev].deviceId &&
                cond->pciDeviceId.vendorId == (uint32_t) gpuInfo.devices[iDev].vendorId &&
                cond->pciDeviceId.revId == (uint32_t) gpuInfo.devices[iDev].revisionId)
            {
                device = &gpuInfo.devices[iDev];
                break;
            }
        }
    }

    if (!device)
        return "Device not found";

    if (result.coreCount)
        *result.coreCount = (uint32_t) device->numCUs;

    if (result.memory)
    {
        result.memory->total = device->localMemoryInBytes;
        result.memory->used = FF_GPU_VMEM_SIZE_UNSET;
    }

    if (result.frequency)
        *result.frequency = (uint32_t) device->coreClock; // Maximum frequency

    if (result.type)
        *result.type = device->isAPU ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;

    return NULL;
}
