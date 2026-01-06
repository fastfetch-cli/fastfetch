#include "gpu.h"

#ifdef FF_HAVE_PCIACCESS

#include "common/io.h"
#include "common/library.h"

#include <pciaccess.h>

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    FF_LIBRARY_LOAD_MESSAGE(pciaccess, "libpciaccess" FF_LIBRARY_EXTENSION, 0)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pciaccess, pci_system_init)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pciaccess, pci_slot_match_iterator_create)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pciaccess, pci_device_next)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pciaccess, pci_system_cleanup)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(pciaccess, pci_iterator_destroy)

    {
        // Requires root access
        // Same behavior can be observed with `cp $(which scanpci) /tmp/ && /tmp/scanpci`
        FF_SUPPRESS_IO();
        if (ffpci_system_init() < 0)
            return "pci_system_init() failed";
    }

    struct pci_device_iterator* iter = ffpci_slot_match_iterator_create(NULL);
    for (struct pci_device* dev = NULL; (dev = ffpci_device_next(iter)); )
    {
        if (dev->device_class >> 16 != 0x03 /*PCI_BASE_CLASS_DISPLAY*/)
            continue;

        uint8_t subclass = (dev->device_class >> 8) & 0xFF;

        if (dev->func > 0 && subclass == 0x80 /*PCI_CLASS_DISPLAY_OTHER*/)
            continue; // Likely an auxiliary display controller (#2034)

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInitStatic(&gpu->vendor, ffGPUGetVendorString(dev->vendor_id));
        ffStrbufInit(&gpu->name);
        ffStrbufInit(&gpu->driver);
        ffStrbufInitStatic(&gpu->platformApi, "libpciaccess");
        ffStrbufInit(&gpu->memoryType);
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = ffGPUPciAddr2Id(dev->domain, dev->bus, dev->dev, dev->func);
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
            ffGPUQueryAmdGpuName(dev->device_id, dev->revision, gpu);

        if (gpu->name.length == 0)
            ffGPUFillVendorAndName(subclass, dev->vendor_id, dev->device_id, gpu);
    }
    ffpci_iterator_destroy(iter);

    ffpci_system_cleanup();
    return NULL;
}

#else

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FF_MAYBE_UNUSED FFlist* gpus)
{
    return "Fastfetch was built without libpciaccess support";
}
#endif
