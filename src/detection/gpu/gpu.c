#include "gpu.h"
#include "detection/internal.h"
#include "detection/vulkan/vulkan.h"

const char* FF_GPU_VENDOR_NAME_APPLE = "Apple";
const char* FF_GPU_VENDOR_NAME_AMD = "AMD";
const char* FF_GPU_VENDOR_NAME_INTEL = "Intel";
const char* FF_GPU_VENDOR_NAME_NVIDIA = "NVIDIA";
const char* FF_GPU_VENDOR_NAME_VMWARE = "VMware";
const char* FF_GPU_VENDOR_NAME_PARALLEL = "Parallel";
const char* FF_GPU_VENDOR_NAME_MICROSOFT = "Microsoft";
const char* FF_GPU_VENDOR_NAME_REDHAT = "RedHat";
const char* FF_GPU_VENDOR_NAME_ORACLE = "Oracle";

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
    // https://devicehunt.com/all-pci-vendors
    if(vendorId == 0x106b)
        return FF_GPU_VENDOR_NAME_APPLE;
    if(arrayContains((const unsigned[]) {0x1002, 0x1022}, vendorId, 2))
        return FF_GPU_VENDOR_NAME_AMD;
    else if(arrayContains((const unsigned[]) {0x03e7, 0x8086, 0x8087}, vendorId, 3))
        return FF_GPU_VENDOR_NAME_INTEL;
    else if(arrayContains((const unsigned[]) {0x0955, 0x10de, 0x12d2}, vendorId, 3))
        return FF_GPU_VENDOR_NAME_NVIDIA;
    else if(arrayContains((const unsigned[]) {0x15ad}, vendorId, 1))
        return FF_GPU_VENDOR_NAME_VMWARE;
    else if(arrayContains((const unsigned[]) {0x1af4}, vendorId, 1))
        return FF_GPU_VENDOR_NAME_REDHAT;
    else if(arrayContains((const unsigned[]) {0x1ab8}, vendorId, 1))
        return FF_GPU_VENDOR_NAME_PARALLEL;
    else if(arrayContains((const unsigned[]) {0x1414}, vendorId, 1))
        return FF_GPU_VENDOR_NAME_MICROSOFT;
    else if(arrayContains((const unsigned[]) {0x108e}, vendorId, 1))
        return FF_GPU_VENDOR_NAME_ORACLE;
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
