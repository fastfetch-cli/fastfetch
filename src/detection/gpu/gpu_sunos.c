#include "gpu.h"
#include "util/stringUtils.h"

#include <libdevinfo.h>

static int walkDevTree(di_node_t node, FF_MAYBE_UNUSED di_minor_t minor, FFlist* gpus)
{
    int* vendorId;
    int* deviceId;
    if (di_prop_lookup_ints(DDI_DEV_T_ANY, node, "vendor-id", &vendorId) > 0
        && di_prop_lookup_ints(DDI_DEV_T_ANY, node, "device-id", &deviceId) > 0)
    {
        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInitS(&gpu->vendor, ffGPUGetVendorString((uint16_t) *vendorId));
        ffStrbufInit(&gpu->name);
        ffStrbufInitS(&gpu->driver, di_driver_name(node));
        ffStrbufInitStatic(&gpu->platformApi, "libdevinfo");
        ffStrbufInit(&gpu->memoryType);
        gpu->index = FF_GPU_INDEX_UNSET;
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = strtoul(di_bus_addr(node), NULL, 16);
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
        {
            int* revId;
            if (di_prop_lookup_ints(DDI_DEV_T_ANY, node, "revision-id", &revId) > 0)
                ffGPUQueryAmdGpuName((uint16_t) *deviceId, (uint8_t) *revId, gpu);
        }

        if (gpu->name.length == 0)
        {
            uint8_t subclass = 0; // assume VGA
            int* classCode;
            if (di_prop_lookup_ints(DDI_DEV_T_ANY, node, "class-code", &classCode) > 0)
                subclass = (uint8_t) (*classCode & 0xFFFF);
            ffGPUFillVendorAndName(subclass, (uint16_t) *vendorId, (uint16_t) *deviceId, gpu);
        }
    }

    return DI_WALK_CONTINUE;
}

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    di_node_t rootNode = di_init("/", DINFOCPYALL);
    if (rootNode == DI_NODE_NIL)
        return "di_init() failed";
    di_walk_minor(rootNode, DDI_NT_DISPLAY, DI_WALK_CLDFIRST, gpus, (void*) walkDevTree);
    di_fini(rootNode);

    return NULL;
}
