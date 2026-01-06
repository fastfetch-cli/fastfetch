#include "gpu.h"
#include "common/io.h"

#include <sys/param.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>
#include <sys/pciio.h>

static inline int pciReadConf(int fd, uint32_t bus, uint32_t device, uint32_t func, uint32_t reg, uint32_t* result)
{
    struct pci_io bdfr = {
        .pi_sel = {
            .pc_bus = bus,
            .pc_dev = device,
            .pc_func = func,
        },
        .pi_reg = reg,
        .pi_width = 4,
    };

    if (ioctl(fd, PCIOCREAD, &bdfr) == -1)
        return -1;

    *result = bdfr.pi_data;
    return 0;
}

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    char pciDevPath[] = "/dev/pci0";
    FF_AUTO_CLOSE_FD int pcifd = open(pciDevPath, O_RDONLY | O_CLOEXEC);
    if (pcifd < 0)
        return "open(\"/dev/pci0\", O_RDONLY | O_CLOEXEC) failed";

    for (uint32_t bus = 0; bus <= 255; bus++)
    {
        for (uint32_t dev = 0; dev < 32; dev++)
        {
            uint32_t maxfuncs = 0;
            for (uint32_t func = 0; func <= maxfuncs; func++)
            {
                uint32_t pciid, pciclass;
                if (pciReadConf(pcifd, bus, dev, func, PCI_ID_REG, &pciid) != 0)
                    continue;

                if (PCI_VENDOR(pciid) == PCI_VENDOR_INVALID || PCI_VENDOR(pciid) == 0)
                    continue;

                if (pciReadConf(pcifd, bus, dev, func, PCI_CLASS_REG, &pciclass) != 0)
                    continue;

                if (func == 0)
                {
                    // For some reason, pciReadConf returns success even for non-existing devices.
                    // So we need to check for `PCI_VENDOR(pciid) == PCI_VENDOR_INVALID` above to filter them out.
                    uint32_t bhlcr;
                    if (pciReadConf(pcifd, bus, dev, 0, PCI_BHLC_REG, &bhlcr) != 0)
                        continue;

                    if (PCI_HDRTYPE_MULTIFN(bhlcr)) maxfuncs = 7;
                }

                if (PCI_CLASS(pciclass) != PCI_CLASS_DISPLAY)
                    continue;

                if (func > 0 && PCI_SUBCLASS(pciclass) == PCI_SUBCLASS_DISPLAY_MISC)
                    continue; // Likely an auxiliary display controller (#2034)

                FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
                ffStrbufInitStatic(&gpu->vendor, ffGPUGetVendorString(PCI_VENDOR(pciid)));
                ffStrbufInit(&gpu->name);
                ffStrbufInit(&gpu->driver);
                ffStrbufInitS(&gpu->platformApi, "/dev/pci0");
                ffStrbufInit(&gpu->memoryType);
                gpu->index = FF_GPU_INDEX_UNSET;
                gpu->temperature = FF_GPU_TEMP_UNSET;
                gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
                gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
                gpu->type = FF_GPU_TYPE_UNKNOWN;
                gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
                gpu->deviceId = ffGPUPciAddr2Id(0, bus, dev, func);
                gpu->frequency = FF_GPU_FREQUENCY_UNSET;

                if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
                    ffGPUQueryAmdGpuName(PCI_PRODUCT(pciid), PCI_REVISION(pciid), gpu);
                if (gpu->name.length == 0)
                    ffGPUFillVendorAndName(PCI_SUBCLASS(pciclass), PCI_VENDOR(pciid), PCI_PRODUCT(pciid), gpu);
            }
        }
    }

    return NULL;
}
