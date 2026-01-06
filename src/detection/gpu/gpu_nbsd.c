#include "gpu.h"
#include "common/io.h"

#include <sys/param.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>
#include <dev/pci/pciio.h>

static inline int pciReadConf(int fd, uint32_t bus, uint32_t device, uint32_t func, uint32_t reg, uint32_t* result)
{
    struct pciio_bdf_cfgreg bdfr = {
        .bus = bus,
        .device = device,
        .function = func,
        .cfgreg = {
            .reg = reg,
        },
    };

    if (ioctl(fd, PCI_IOC_BDF_CFGREAD, &bdfr) == -1)
        return -1;

    *result = bdfr.cfgreg.val;
    return 0;
}

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    char pciDevPath[] = "/dev/pciXXX";

    for (uint32_t idev = 0; idev <= 255; idev++)
    {
        snprintf(pciDevPath + strlen("/dev/pci"), 4, "%u", idev);

        FF_AUTO_CLOSE_FD int pcifd = open(pciDevPath, O_RDONLY | O_CLOEXEC);
        if (pcifd < 0)
        {
            if (errno == ENOENT)
                break; // No more /dev/pciN devices
            return "open(\"/dev/pciN\", O_RDONLY | O_CLOEXEC) failed";
        }

        struct pciio_businfo businfo;
        if (ioctl(pcifd, PCI_IOC_BUSINFO, &businfo) != 0)
            continue;

        uint32_t bus = businfo.busno;
        for (uint32_t dev = 0; dev < businfo.maxdevs; dev++)
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
                ffStrbufInitS(&gpu->platformApi, pciDevPath);
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

                struct pciio_drvname drvname = {
                    .device = dev,
                    .function = func,
                };
                if (ioctl(pcifd, PCI_IOC_DRVNAME, &drvname) == 0)
                    ffStrbufInitS(&gpu->driver, drvname.name);
            }
        }
    }

    return NULL;
}
