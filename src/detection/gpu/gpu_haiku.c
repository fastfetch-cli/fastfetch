#include "gpu.h"
#include "common/io/io.h"

#include <private/drivers/poke.h>

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    FF_AUTO_CLOSE_FD int pokefd = open(POKE_DEVICE_FULLNAME, O_RDWR | O_CLOEXEC);
    if (pokefd < 0) return "open(POKE_DEVICE_FULLNAME) failed";

    pci_info dev;
    pci_info_args cmd = {
        .signature = POKE_SIGNATURE,
        .info = &dev,
    };

    for (cmd.index = 0; ioctl(pokefd, POKE_GET_NTH_PCI_INFO, &cmd, sizeof(cmd)) == B_OK && cmd.status == B_OK; ++cmd.index)
    {
        if (dev.class_base != 0x03 /*PCI_BASE_CLASS_DISPLAY*/)
            continue;

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInitStatic(&gpu->vendor, ffGPUGetVendorString(dev.vendor_id));
        ffStrbufInit(&gpu->name);
        ffStrbufInit(&gpu->driver);
        ffStrbufInitStatic(&gpu->platformApi, POKE_DEVICE_FULLNAME);
        ffStrbufInit(&gpu->memoryType);
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = ((uint64_t) dev.bus << 4) | ((uint64_t) dev.device << 2) | (uint64_t) dev.function;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
            ffGPUQueryAmdGpuName(dev.device_id, dev.revision, gpu);

        if (gpu->name.length == 0)
            ffGPUFillVendorAndName(dev.class_sub, dev.vendor_id, dev.device_id, gpu);
    }

    return NULL;
}
