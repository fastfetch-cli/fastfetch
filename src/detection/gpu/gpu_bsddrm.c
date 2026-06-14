#if FF_HAVE_DRM

    #include "gpu.h"
    #include "common/strutil.h"
    #include "common/io.h"

    #include <drm.h>
    #include <sys/ioctl.h>
    #if __FreeBSD__
        #include <sys/sysctl.h>
        #include <inttypes.h>
        #include <sys/pciio.h>
        #include <fcntl.h>
        #if __has_include(<dev/pci/pcireg.h>)
            #include <dev/pci/pcireg.h> // FreeBSD
        #else
            #include <bus/pci/pcireg.h> // DragonFly
        #endif
    #endif

// https://github.com/freebsd/drm-kmod/blob/8fea1f06b3ac3fa24217e24ba7b2133abad705a9/include/uapi/drm/drm.h#L1095
struct drm_pciinfo {
    uint16_t domain;
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t subvendor_id;
    uint16_t subdevice_id;
    uint8_t revision_id;
};

    #define DRM_IOCTL_GET_PCIINFO DRM_IOR(0x15, struct drm_pciinfo)

static const char* drmGetPciinfo(int drmfd, const char* drmId, struct drm_pciinfo* result) {
    #if !__FreeBSD__
    FF_UNUSED(drmId);
    if (ioctl(drmfd, DRM_IOCTL_GET_PCIINFO, result) < 0) {
        return "ioctl(DRM_IOCTL_GET_PCIINFO) failed";
    }
    #else
    FF_UNUSED(drmfd);
    char requestBuf[64];
    snprintf(requestBuf, ARRAY_SIZE(requestBuf), "hw.dri.%s.busid", drmId);
    char pciidBuf[64];
    size_t pciidLen = ARRAY_SIZE(pciidBuf);
    if (sysctlbyname(requestBuf, pciidBuf, &pciidLen, NULL, 0) < 0) {
        return "sysctlbyname(hw.dri.%s.busid) failed";
    }
    // hw.dri.0.busid: pci:0000:01:00.0
    struct pci_match_conf match = {
        .pc_class = PCIC_DISPLAY,
        .flags = PCI_GETCONF_MATCH_CLASS | PCI_GETCONF_MATCH_DOMAIN | PCI_GETCONF_MATCH_BUS | PCI_GETCONF_MATCH_DEV | PCI_GETCONF_MATCH_FUNC,
    };
    if (sscanf(pciidBuf, "pci:%" SCNx32 ":%" SCNx8 ":%" SCNx8 ".%" SCNx8, &match.pc_sel.pc_domain, &match.pc_sel.pc_bus, &match.pc_sel.pc_dev, &match.pc_sel.pc_func) != 4) {
        return "Invalid PCI busid found";
    }

    FF_AUTO_CLOSE_FD int pcifd = open("/dev/pci", O_RDONLY | O_CLOEXEC);
    if (pcifd < 0) {
        return "open(\"/dev/pci\", O_RDONLY | O_CLOEXEC, 0) failed";
    }

    struct pci_conf conf;
    struct pci_conf_io pcio = {
        .pat_buf_len = sizeof(match),
        .num_patterns = 1,
        .patterns = &match,
        .match_buf_len = sizeof(conf),
        .matches = &conf,
    };

    if (ioctl(pcifd, PCIOCGETCONF, &pcio) < 0) {
        perror("ioctl");
        return "ioctl(pcifd, PCIOCGETCONF, &pc) failed";
    }

    if (pcio.status == PCI_GETCONF_ERROR) {
        return "ioctl(pcifd, PCIOCGETCONF, &pc) returned error";
    }

    if (pcio.num_matches != 1) {
        return "ioctl(pcifd, PCIOCGETCONF, &pc) returned no results";
    }

    *result = (struct drm_pciinfo){
        .domain = (uint16_t) conf.pc_sel.pc_domain,
        .bus = conf.pc_sel.pc_bus,
        .dev = conf.pc_sel.pc_dev,
        .func = conf.pc_sel.pc_func,
        .vendor_id = conf.pc_vendor,
        .device_id = conf.pc_device,
        .subvendor_id = conf.pc_subvendor,
        .subdevice_id = conf.pc_subdevice,
        .revision_id = conf.pc_revid,
    };

        // This sysctl can also be useful
        // dev.drm.0.PCI_ID: 10de:2782
    #endif

    return NULL;
}

const char* ffGPUDetectByDrmBSD(const FFGPUOptions* options, FFlist* gpus) {
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
        if (entry->d_name[0] == '.' || !ffStrStartsWith(entry->d_name, "card")) {
            continue;
        }

        ffStrbufAppendS(&pathBuf, entry->d_name);

        FF_AUTO_CLOSE_FD int fd = open(pathBuf.chars, O_RDWR | O_CLOEXEC);
        ffStrbufSubstrBefore(&pathBuf, pathLen);
        if (fd < 0) {
            continue;
        }

        // Currently, DRM_BUS_PCI is the only bus type supported by drm-kmod on BSD (hard-coded in libdrm source tree)
        struct drm_pciinfo pciInfo;
        if (drmGetPciinfo(fd, entry->d_name + strlen("card"), &pciInfo) != NULL) {
            continue;
        }

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
        gpu->pcieSpeed = FF_GPU_PCIE_SPEED_UNSET;

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
            ffGPUDetectDriverSpecific(options, gpu, (FFGpuDriverPciBusId){
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

#else

const char* ffGPUDetectByDrmBSD(const FFGPUOptions* options, FFlist* gpus) {
    FF_UNUSED(options, gpus);
    return "Fastfetch was built without libdrm support";
}

#endif // __FreeBSD__ || __OpenBSD__
