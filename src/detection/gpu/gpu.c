#include "gpu.h"
#include "detection/internal.h"
#include "detection/vulkan/vulkan.h"

const char* FF_GPU_VENDOR_NAME_APPLE = "Apple";
const char* FF_GPU_VENDOR_NAME_AMD = "AMD";
const char* FF_GPU_VENDOR_NAME_INTEL = "Intel";
const char* FF_GPU_VENDOR_NAME_NVIDIA = "NVIDIA";

static inline bool arrayContains(const unsigned arr[], unsigned vendorId, unsigned length)
{
    for (unsigned i = 0; i < length; ++i)
    {
        if (arr[i] == vendorId)
            return true;
    }
    return false;
}

const char* ffGetGPUVendorString(unsigned vendorId)
{
    if(vendorId == 0x106b)
        return FF_GPU_VENDOR_NAME_APPLE;
    if(arrayContains((const unsigned[]) {0x1002, 0x1022}, vendorId, 2))
        return FF_GPU_VENDOR_NAME_AMD;
    else if(arrayContains((const unsigned[]) {0x03e7, 0x8086, 0x8087}, vendorId, 3))
        return FF_GPU_VENDOR_NAME_INTEL;
    else if(arrayContains((const unsigned[]) {0x0955, 0x10de, 0x12d2}, vendorId, 3))
        return FF_GPU_VENDOR_NAME_NVIDIA;
    return NULL;
}

const char* ffDetectGPU(const FFGPUOptions* options, FFlist* result)
{
    if (!options->forceVulkan)
    {
        const char* error = ffDetectGPUImpl(options, result);
        if (!error) return NULL;
    }
    FFVulkanResult* vulkan = ffDetectVulkan();
    if (vulkan->error) return "GPU detection failed";
    ffListDestroy(result);
    ffListInitMove(result, &vulkan->gpus);

    return NULL;
}
