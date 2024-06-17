#include "gpu.h"
#include "common/properties.h"
#include "common/io/io.h"

#include <pciaccess.h>

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    {
        // Requires root access
        // Same behavior can be observed with `cp $(which scanpci) /tmp/ && /tmp/scanpci`
        FF_SUPPRESS_IO();
        if (pci_system_init() < 0)
            return "pci_system_init() failed";
    }

    struct pci_device_iterator* iter = pci_slot_match_iterator_create(NULL);

    FF_STRBUF_AUTO_DESTROY pciids = ffStrbufCreate();
    for (struct pci_device* dev = NULL; (dev = pci_device_next( iter )); )
    {
        if (dev->device_class >> 16 != 0x03 /*PCI_BASE_CLASS_DISPLAY*/)
            continue;

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInitStatic(&gpu->vendor, ffGetGPUVendorString(dev->vendor_id));
        ffStrbufInit(&gpu->name);
        ffStrbufInit(&gpu->driver);
        ffStrbufInit(&gpu->platformApi);
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = ((uint64_t) dev->domain << 6) | ((uint64_t) dev->bus << 4) | (dev->dev << 2) | dev->func;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
        {
            char query[32];
            snprintf(query, sizeof(query), "%X,\t%X,", (unsigned) dev->device_id, (unsigned) dev->revision);
            ffParsePropFileData("libdrm/amdgpu.ids", query, &gpu->name);
        }

        if (gpu->name.length == 0)
        {
            if (pciids.length == 0)
                ffReadFileBuffer(FASTFETCH_TARGET_DIR_ROOT "/usr/share/hwdata/pci.ids", &pciids);;
            ffGPUParsePciIds(&pciids, (dev->device_class >> 8) & 8, dev->vendor_id, dev->device_id, gpu);
        }
    }

    pci_system_cleanup();

    return NULL;
}
