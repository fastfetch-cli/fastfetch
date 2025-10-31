#include "gpu.h"
#include "common/io/io.h"

#include <sys/param.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pci.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>
#include <dev/pci/pciio.h>

const char* ffDetectGPUImpl(const FFGPUOptions* options, FFlist* gpus)
{
    FF_AUTO_CLOSE_FD int pcifd = open("/dev/pci0", O_RDONLY | O_CLOEXEC);
    if (pcifd < 0)
        return "open(\"/dev/pci0\", O_RDONLY | O_CLOEXEC, 0) failed";

    struct pciio_businfo businfo;
    if (ioctl(pcifd, PCI_IOC_BUSINFO, &businfo) != 0)
        return "ioctl(pcifd, PCI_IOC_BUSINFO, &businfo) failed";

    for (uint32_t bus = 0; bus <= 255; bus++)
    {
        for (uint32_t dev = 0; dev < businfo.maxdevs; dev++)
        {
            pcireg_t bhlcr;
            if (pcibus_conf_read(pcifd, bus, dev, 0, PCI_BHLC_REG, &bhlcr) != 0)
                continue;

            uint32_t maxfunc = PCI_HDRTYPE_MULTIFN(bhlcr) ? 7 : 0;
            for (uint32_t func = 0; func <= maxfunc; func++)
            {
                pcireg_t pciid, pciclass;
                if (pcibus_conf_read(pcifd, bus, dev, func, PCI_ID_REG, &pciid) != 0)
                    continue;

                if (PCI_VENDOR(pciid) == PCI_VENDOR_INVALID || PCI_VENDOR(pciid) == 0)
                    continue;

                if (pcibus_conf_read(pcifd, bus, dev, func, PCI_CLASS_REG, &pciclass) != 0)
                    continue;

                if (PCI_CLASS(pciclass) != PCI_CLASS_DISPLAY)
                    continue;

                if (func > 0 && PCI_SUBCLASS(pciclass) == PCI_SUBCLASS_DISPLAY_MISC)
                    continue; // Likely an auxiliary display controller (#2034)

                char drvname[32];
                if (pci_drvnameonbus(pcifd, bus, dev, func, drvname, ARRAY_SIZE(drvname)) != 0)
                    drvname[0] = '\0';

                FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
                ffStrbufInitStatic(&gpu->vendor, ffGPUGetVendorString(PCI_VENDOR(pciid)));
                ffStrbufInit(&gpu->name);
                ffStrbufInitS(&gpu->driver, drvname);
                ffStrbufInitStatic(&gpu->platformApi, "/dev/pci0");
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
