#include "gpu_driver_specific.h"

#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <sys/pciio.h>
#include <fcntl.h>
#if __has_include(<dev/pci/pcireg.h>)
    #include <dev/pci/pcireg.h> // FreeBSD
#else
    #include <bus/pci/pcireg.h> // DragonFly
#endif

static void fillGPUTypeGeneric(FFGPUResult* gpu)
{
    if (gpu->type == FF_GPU_TYPE_UNKNOWN)
    {
        if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_NVIDIA)
        {
            if (ffStrbufStartsWithIgnCaseS(&gpu->name, "GeForce") ||
                ffStrbufStartsWithIgnCaseS(&gpu->name, "Quadro") ||
                ffStrbufStartsWithIgnCaseS(&gpu->name, "Tesla"))
                gpu->type = FF_GPU_TYPE_DISCRETE;
        }
        else if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_MTHREADS)
        {
            if (ffStrbufStartsWithIgnCaseS(&gpu->name, "MTT "))
                gpu->type = FF_GPU_TYPE_DISCRETE;
        }
        else if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_INTEL)
        {
            // 0000:00:02.0 is reserved for Intel integrated graphics
            gpu->type = gpu->deviceId == 20 ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;
        }
    }
}

#if FF_HAVE_DRM
#include "common/library.h"
#include "util/stringUtils.h"

#include <xf86drm.h>

static const char* detectByDrm(const FFGPUOptions* options, FFlist* gpus)
{
    FF_LIBRARY_LOAD(libdrm, "dlopen libdrm" FF_LIBRARY_EXTENSION " failed", "libdrm" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmGetDevices)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmFreeDevices)

    drmDevicePtr devices[64];
    int nDevices = ffdrmGetDevices(devices, ARRAY_SIZE(devices));
    if (nDevices < 0)
        return "drmGetDevices() failed";

    for (int iDev = 0; iDev < nDevices; ++iDev)
    {
        drmDevice* dev = devices[iDev];

        if (!(dev->available_nodes & (1 << DRM_NODE_PRIMARY)))
            continue;

        const char* path = dev->nodes[DRM_NODE_PRIMARY];

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInit(&gpu->vendor);
        ffStrbufInit(&gpu->name);
        ffStrbufInit(&gpu->driver);
        ffStrbufInitS(&gpu->platformApi, path);
        ffStrbufInit(&gpu->memoryType);
        gpu->index = FF_GPU_INDEX_UNSET;
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = 0;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        switch (dev->bustype)
        {
        case DRM_BUS_PCI:
            ffStrbufInitStatic(&gpu->vendor, ffGPUGetVendorString(dev->deviceinfo.pci->vendor_id));
            gpu->deviceId = (dev->businfo.pci->domain * 100000ull) + (dev->businfo.pci->bus * 1000ull) + (dev->businfo.pci->dev * 10ull) + dev->businfo.pci->func;
            break;
        case DRM_BUS_HOST1X:
            ffStrbufSetS(&gpu->name, dev->deviceinfo.host1x->compatible[0]);
            gpu->type = FF_GPU_TYPE_INTEGRATED;
            break;
        case DRM_BUS_PLATFORM:
            ffStrbufSetS(&gpu->name, dev->deviceinfo.platform->compatible[0]);
            gpu->type = FF_GPU_TYPE_INTEGRATED;
            break;
        case DRM_BUS_USB:
            ffStrbufSetF(&gpu->name, "USB Device (%u-%u)", dev->deviceinfo.usb->vendor, dev->deviceinfo.usb->product);
            gpu->type = FF_GPU_TYPE_DISCRETE;
            break;
        }

        FF_AUTO_CLOSE_FD int fd = open(path, O_RDONLY | O_CLOEXEC);
        if (fd < 0) continue;

        char driverName[64];
        driverName[0] = '\0';
        struct drm_version ver = {
            .name = driverName,
            .name_len = ARRAY_SIZE(driverName),
        };
        if (ioctl(fd, DRM_IOCTL_VERSION, &ver) == 0)
        {
            driverName[ver.name_len] = '\0';
            ffStrbufSetF(&gpu->driver, "%s %d.%d.%d", ver.name, ver.version_major, ver.version_minor, ver.version_patchlevel);
        }

        if (ffStrStartsWith(driverName, "i915"))
            ffDrmDetectI915(gpu, fd);
        else if (ffStrStartsWith(driverName, "amdgpu"))
            ffDrmDetectAmdgpu(options, gpu, dev->nodes[DRM_NODE_RENDER]);
        else if (ffStrStartsWith(driverName, "radeon"))
            ffDrmDetectRadeon(options, gpu, dev->nodes[DRM_NODE_RENDER]);
        else if (ffStrStartsWith(driverName, "xe"))
            ffDrmDetectXe(gpu, fd);
        else if (ffStrStartsWith(driverName, "asahi"))
            ffDrmDetectAsahi(gpu, fd);
        else if (ffStrStartsWith(driverName, "nouveau"))
            ffDrmDetectNouveau(gpu, fd);
        else if (dev->bustype == DRM_BUS_PCI)
        {
            ffGPUDetectDriverSpecific(options, gpu, (FFGpuDriverPciBusId) {
                .domain = (uint32_t) dev->businfo.pci->domain,
                .bus = dev->businfo.pci->bus,
                .device = dev->businfo.pci->dev,
                .func = dev->businfo.pci->func,
            });
        }

        if (gpu->name.length == 0)
        {
            if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
                ffGPUQueryAmdGpuName(dev->deviceinfo.pci->device_id, dev->deviceinfo.pci->revision_id, gpu);
            if (gpu->name.length == 0)
                ffGPUFillVendorAndName(0, dev->deviceinfo.pci->vendor_id, dev->deviceinfo.pci->device_id, gpu);
        }

        fillGPUTypeGeneric(gpu);
    }

    ffdrmFreeDevices(devices, nDevices);

    return NULL;
}
#endif

static const char* detectByPci(const FFGPUOptions* options, FFlist* gpus)
{
    FF_AUTO_CLOSE_FD int fd = open("/dev/pci", O_RDONLY | O_CLOEXEC);
    if (fd < 0)
        return "open(\"/dev/pci\", O_RDONLY | O_CLOEXEC, 0) failed";

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

    if (ioctl(fd, PCIOCGETCONF, &pcio) < 0)
        return "ioctl(fd, PCIOCGETCONF, &pc) failed";

    if (pcio.status == PCI_GETCONF_ERROR)
        return "ioctl(fd, PCIOCGETCONF, &pc) returned error";

    for (uint32_t i = 0; i < pcio.num_matches; ++i)
    {
        struct pci_conf* pc = &confs[i];

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
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
        gpu->deviceId = (pc->pc_sel.pc_domain * 100000ull) + (pc->pc_sel.pc_bus * 1000ull) + (pc->pc_sel.pc_dev * 10ull) + pc->pc_sel.pc_func;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        ffGPUDetectDriverSpecific(options, gpu, (FFGpuDriverPciBusId) {
            .domain = (uint32_t) pc->pc_sel.pc_domain,
            .bus = pc->pc_sel.pc_bus,
            .device = pc->pc_sel.pc_dev,
            .func = pc->pc_sel.pc_func,
        });

        if (gpu->name.length == 0)
        {
            if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
                ffGPUQueryAmdGpuName(pc->pc_device, pc->pc_revid, gpu);
            if (gpu->name.length == 0)
                ffGPUFillVendorAndName(pc->pc_subclass, pc->pc_vendor, pc->pc_device, gpu);
        }

        fillGPUTypeGeneric(gpu);
    }

    return NULL;
}

const char* ffDetectGPUImpl(const FFGPUOptions* options, FFlist* gpus)
{
    #if FF_HAVE_DRM
    if (options->detectionMethod == FF_GPU_DETECTION_METHOD_AUTO)
    {
        detectByDrm(options, gpus);
        if (gpus->length > 0) return NULL;
    }
    #endif

    return detectByPci(options, gpus);
}
