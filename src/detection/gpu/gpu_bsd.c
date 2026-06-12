#include "gpu.h"
#include "gpu_driver_specific.h"

#include "common/io.h"

#include <sys/pciio.h>
#include <fcntl.h>
#if __has_include(<dev/pci/pcireg.h>)
    #include <dev/pci/pcireg.h> // FreeBSD
#else
    #include <bus/pci/pcireg.h> // DragonFly
#endif

static const char* detectByPci(const FFGPUOptions* options, FFlist* gpus) {
    FF_AUTO_CLOSE_FD int fd = open("/dev/pci", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return "open(\"/dev/pci\", O_RDONLY | O_CLOEXEC, 0) failed";
    }

    struct pci_conf confs[128];
    struct pci_match_conf match = {
        .pc_class = PCIC_DISPLAY,
        .flags = PCI_GETCONF_MATCH_CLASS,
    };
    struct pci_conf_io pcio = {
        .pat_buf_len = sizeof(match),
        .num_patterns = 1,
        .patterns = &match,
        .match_buf_len = sizeof(confs),
        .matches = confs,
    };

    if (ioctl(fd, PCIOCGETCONF, &pcio) < 0) {
        return "ioctl(fd, PCIOCGETCONF, &pc) failed";
    }

    if (pcio.status == PCI_GETCONF_ERROR) {
        return "ioctl(fd, PCIOCGETCONF, &pc) returned error";
    }

    for (uint32_t i = 0; i < pcio.num_matches; ++i) {
        struct pci_conf* pc = &confs[i];

        if (pc->pc_sel.pc_func > 0 && pc->pc_subclass == 0x80 /*PCI_CLASS_DISPLAY_OTHER*/) {
            continue; // Likely an auxiliary display controller (#2034)
        }

        FFGPUResult* gpu = FF_LIST_ADD(FFGPUResult, *gpus);
        ffStrbufInitStatic(&gpu->vendor, ffGPUGetVendorString(pc->pc_vendor));
        ffStrbufInit(&gpu->name);
        ffStrbufInitS(&gpu->driver, pc->pd_name);
        ffStrbufInitStatic(&gpu->platformApi, "/dev/pci");
        ffStrbufInit(&gpu->memoryType);
        gpu->index = FF_GPU_INDEX_UNSET;
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = ffGPUPciAddr2Id(pc->pc_sel.pc_domain, pc->pc_sel.pc_bus, pc->pc_sel.pc_dev, pc->pc_sel.pc_func);
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        ffGPUDetectDriverSpecific(options, gpu, (FFGpuDriverPciBusId) {
                                                    .domain = (uint32_t) pc->pc_sel.pc_domain,
                                                    .bus = pc->pc_sel.pc_bus,
                                                    .device = pc->pc_sel.pc_dev,
                                                    .func = pc->pc_sel.pc_func,
                                                });

        if (gpu->name.length == 0) {
            if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD) {
                ffGPUQueryAmdGpuName(pc->pc_device, pc->pc_revid, gpu);
            }
            if (gpu->name.length == 0) {
                ffGPUFillVendorAndName(pc->pc_subclass, pc->pc_vendor, pc->pc_device, gpu);
            }
        }

        ffGPUFillVendorByDeviceName(gpu);
    }

    return NULL;
}

const char* ffDetectGPUImpl(const FFGPUOptions* options, FFlist* gpus) {
#if FF_HAVE_DRM
    if (options->detectionMethod == FF_GPU_DETECTION_METHOD_AUTO) {
        ffGPUDetectByDrmBSD(options, gpus);
        if (gpus->length > 0) {
            return NULL;
        }
    }
#endif

    return detectByPci(options, gpus);
}
