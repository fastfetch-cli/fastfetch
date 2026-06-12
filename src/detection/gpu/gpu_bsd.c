#include "gpu_driver_specific.h"

#include "common/io.h"
#include "common/mallocHelper.h"

#include <sys/pciio.h>
#include <fcntl.h>
#if __has_include(<dev/pci/pcireg.h>)
    #include <dev/pci/pcireg.h> // FreeBSD
#else
    #include <bus/pci/pcireg.h> // DragonFly
#endif

#if FF_HAVE_DRM
    #include "common/library.h"
    #include "common/strutil.h"

    #include <drm.h>

    // https://github.com/freebsd/drm-kmod/blob/8fea1f06b3ac3fa24217e24ba7b2133abad705a9/include/uapi/drm/drm.h#L1095
    struct drm_pciinfo {
        uint16_t	domain;
        uint8_t		bus;
        uint8_t		dev;
        uint8_t		func;
        uint16_t	vendor_id;
        uint16_t	device_id;
        uint16_t	subvendor_id;
        uint16_t	subdevice_id;
        uint8_t		revision_id;
    };

    #define DRM_IOCTL_GET_PCIINFO	DRM_IOR(0x15, struct drm_pciinfo)

static const char* detectByDrm(const FFGPUOptions* options, FFlist* gpus) {
    const char* drmDirPath = "/dev/dri/";
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(drmDirPath);
    if (dirp == NULL) {
        return "opendir(/dev/dri/) failed";
    }

    FF_STRBUF_AUTO_DESTROY pathBuf = ffStrbufCreateA(64);
    ffStrbufAppendS(&pathBuf, drmDirPath);
    uint32_t pathLen = pathBuf.length;

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_name[0] == '.' || !ffStrStartsWith(entry->d_name, "render")) {
            continue;
        }

        ffStrbufAppendS(&pathBuf, entry->d_name);

        FF_AUTO_CLOSE_FD int fd = open(pathBuf.chars, O_RDWR | O_CLOEXEC);
        ffStrbufSubstrBefore(&pathBuf, pathLen);
        if (fd < 0) {
            continue;
        }

        // Currently, DRM_BUS_PCI is the only bus type supported by drm-kmod on BSD (hard-coded in libdrm source tree)
        struct drm_pciinfo pciInfo = {};
        if (ioctl(fd, DRM_IOCTL_GET_PCIINFO, &pciInfo) < 0) {
            continue;
        }
        // dev.drm.0.PCI_ID: 10de:2782
        // dev.drm.128.PCI_ID: 10de:2782
        // dev.drm.drm_debug_persist: 0
        // dev.drm.skip_ddb: 0
        // dev.drm.__drm_debug: 0

        // hw.dri.timestamp_precision: 20
        // hw.dri.vblank_offdelay: 5000
        // hw.dri.0.modesetting: 1
        // hw.dri.0.busid: pci:0000:01:00.0
        // hw.dri.0.vblank:
        // crtc ref count    last     enabled inmodeset
        // hw.dri.0.clients:
        // a dev            pid   uid      magic     ioctls
        // y drm/128      101655     0          0          0
        // n drm/128      101655     0          0          0
        // y drm/128      101234     0          0          0
        // n drm/128      101234     0          0          0
        // n drm/128      101784     0          0          0
        // n drm/128      107428     0          0          0
        // y drm/128      100891     0          0          0
        // y drm/128      100891     0          0          0

        // hw.dri.0.name: nvidia-drm 0x85
        // hw.dri.dp_aux_i2c_transfer_size: 16
        // hw.dri.dp_aux_i2c_speed_khz: 10
        // hw.dri.sched_policy: 1
        // hw.dri.drm_fbdev_overalloc: 100
        // hw.dri.fbdev_emulation: 1
        // hw.dri.timestamp_precision_usec: 20
        // hw.dri.vblankoffdelay: 5000
        // hw.dri.poll: 1
        // hw.dri.debug: 0
        // hw.dri.drm_debug_persist: 0
        // hw.dri.skip_ddb: 0
        // hw.dri.__drm_debug: 0
        // hw.dri.edid_fixup: 6


        FFGPUResult* gpu = FF_LIST_ADD(FFGPUResult, *gpus);
        ffStrbufInitStatic(&gpu->vendor, ffGPUGetVendorString(pciInfo.vendor_id));
        ffStrbufInit(&gpu->name);
        ffStrbufInit(&gpu->driver);
        ffStrbufInitF(&gpu->platformApi, "/dev/dri/%s", entry->d_name);
        ffStrbufInit(&gpu->memoryType);
        gpu->index = FF_GPU_INDEX_UNSET;
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = ffGPUPciAddr2Id(pciInfo.domain, pciInfo.bus, pciInfo.dev, pciInfo.func);
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        char driverName[64];
        driverName[0] = '\0';
        struct drm_version ver = {
            .name = driverName,
            .name_len = ARRAY_SIZE(driverName),
        };
        if (ioctl(fd, DRM_IOCTL_VERSION, &ver) == 0) {
            driverName[ver.name_len] = '\0';
            ffStrbufSetF(&gpu->driver, "%s %d.%d.%d", ver.name, ver.version_major, ver.version_minor, ver.version_patchlevel);
        }

        if (ffStrStartsWith(driverName, "i915")) {
            ffDrmDetectI915(gpu, fd);
        } else if (ffStrStartsWith(driverName, "amdgpu")) {
            uint32_t primaryNodeId = (uint32_t) strtoul(entry->d_name + strlen("card"), NULL, 10);
            FF_STRBUF_AUTO_DESTROY renderNode = ffStrbufCreateF("/dev/dri/renderD%d", primaryNodeId + 128);
            ffDrmDetectAmdgpu(options, gpu, renderNode.chars);
        } else if (ffStrStartsWith(driverName, "radeon")) {
            uint32_t primaryNodeId = (uint32_t) strtoul(entry->d_name + strlen("card"), NULL, 10);
            FF_STRBUF_AUTO_DESTROY renderNode = ffStrbufCreateF("/dev/dri/renderD%d", primaryNodeId + 128);
            ffDrmDetectRadeon(options, gpu, renderNode.chars);
        } else if (ffStrStartsWith(driverName, "xe")) {
            ffDrmDetectXe(gpu, fd);
        } else if (ffStrStartsWith(driverName, "asahi")) {
            ffDrmDetectAsahi(gpu, fd);
        } else if (ffStrStartsWith(driverName, "nouveau")) {
            ffDrmDetectNouveau(gpu, fd);
        } else {
            ffGPUDetectDriverSpecific(options, gpu, (FFGpuDriverPciBusId) {
                                                        .domain = pciInfo.domain,
                                                        .bus = pciInfo.bus,
                                                        .device = pciInfo.dev,
                                                        .func = pciInfo.func,
                                                    });
        }

        if (gpu->name.length == 0) {
            if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD) {
                ffGPUQueryAmdGpuName(pciInfo.device_id, pciInfo.revision_id, gpu);
            }
            if (gpu->name.length == 0) {
                ffGPUFillVendorAndName(0, pciInfo.vendor_id, pciInfo.device_id, gpu);
            }
        }

        ffGPUFillVendorByDeviceName(gpu);
    }

    return NULL;
}
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
        detectByDrm(options, gpus);
        if (gpus->length > 0) {
            return NULL;
        }
    }
#endif

    return detectByPci(options, gpus);
}
