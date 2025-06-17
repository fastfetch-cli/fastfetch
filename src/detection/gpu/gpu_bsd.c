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

#include "common/library.h"
#include "util/stringUtils.h"
#include <xf86drm.h>
#include <i915_drm.h>

#ifdef FF_HAVE_DRM_AMDGPU
#include <amdgpu.h>
#include <amdgpu_drm.h>
#include <fcntl.h>
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
static const char* drmDetectAmdSpecific(const FFGPUOptions* options, FFGPUResult* gpu, const char* renderPath)
{
#if FF_HAVE_DRM_AMDGPU
    FF_LIBRARY_LOAD(libdrm, "dlopen libdrm_amdgpu" FF_LIBRARY_EXTENSION " failed", "libdrm_amdgpu" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_device_initialize)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_get_marketing_name)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_query_gpu_info)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_query_sensor_info)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_query_heap_info)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_device_deinitialize)

    FF_AUTO_CLOSE_FD int fd = open(renderPath, O_RDONLY);
    if (fd < 0) return "Failed to open DRM device";

    amdgpu_device_handle handle;
    uint32_t majorVersion, minorVersion;
    if (ffamdgpu_device_initialize(fd, &majorVersion, &minorVersion, &handle) < 0)
        return "Failed to initialize AMDGPU device";

    ffStrbufAppendF(&gpu->driver, " %u.%u", (unsigned) majorVersion, (unsigned) minorVersion);

    uint32_t value;

    if (options->temp)
    {
        if (ffamdgpu_query_sensor_info(handle, AMDGPU_INFO_SENSOR_GPU_TEMP, sizeof(value), &value) >= 0)
            gpu->temperature = value / 1000.;
    }

    ffStrbufSetS(&gpu->name, ffamdgpu_get_marketing_name(handle));

    struct amdgpu_gpu_info gpuInfo;
    if (ffamdgpu_query_gpu_info(handle, &gpuInfo) >= 0)
    {
        gpu->coreCount = (int32_t) gpuInfo.cu_active_number;
        gpu->frequency = (uint32_t) (gpuInfo.max_engine_clk / 1000u);
        gpu->index = FF_GPU_INDEX_UNSET;
        gpu->type = gpuInfo.ids_flags & AMDGPU_IDS_FLAGS_FUSION ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;
#define FF_VRAM_CASE(name, value) case value /* AMDGPU_VRAM_TYPE_ ## name */: ffStrbufSetStatic(&gpu->memoryType, #name); break
        switch (gpuInfo.vram_type)
        {
            FF_VRAM_CASE(UNKNOWN, 0);
            FF_VRAM_CASE(GDDR1, 1);
            FF_VRAM_CASE(DDR2, 2);
            FF_VRAM_CASE(GDDR3, 3);
            FF_VRAM_CASE(GDDR4, 4);
            FF_VRAM_CASE(GDDR5, 5);
            FF_VRAM_CASE(HBM, 6);
            FF_VRAM_CASE(DDR3, 7);
            FF_VRAM_CASE(DDR4, 8);
            FF_VRAM_CASE(GDDR6, 9);
            FF_VRAM_CASE(DDR5, 10);
            FF_VRAM_CASE(LPDDR4, 11);
            FF_VRAM_CASE(LPDDR5, 12);
        default:
            ffStrbufAppendF(&gpu->memoryType, "Unknown (%u)", gpuInfo.vram_type);
            break;
        }

        struct amdgpu_heap_info heapInfo;
        if (ffamdgpu_query_heap_info(handle, AMDGPU_GEM_DOMAIN_VRAM, 0, &heapInfo) >= 0)
        {
            if (gpu->type == FF_GPU_TYPE_DISCRETE)
            {
                gpu->dedicated.total = heapInfo.heap_size;
                gpu->dedicated.used = heapInfo.heap_usage;
            }
            else
            {
                gpu->shared.total = heapInfo.heap_size;
                gpu->shared.used = heapInfo.heap_usage;
            }
        }
    }

    if (ffamdgpu_query_sensor_info(handle, AMDGPU_INFO_SENSOR_GPU_LOAD, sizeof(value), &value) >= 0)
        gpu->coreUsage = value;

    ffamdgpu_device_deinitialize(handle);

    return NULL;
#else
    FF_UNUSED(options, gpu, drmKey, buffer);
    return "Fastfetch is compiled without libdrm support";
#endif
}

static const char* drmDetectIntelSpecific(FFGPUResult* gpu, int fd)
{
    {
        int value;
        drm_i915_getparam_t getparam = { .param = I915_PARAM_EU_TOTAL, .value = &value };
        if (ioctl(fd, DRM_IOCTL_I915_GETPARAM, &getparam) >= 0)
            gpu->coreCount = value;
    }
    {
        struct drm_i915_query_item queryItem = {
            .query_id = DRM_I915_QUERY_MEMORY_REGIONS,
        };
        struct drm_i915_query query = {
            .items_ptr = (uintptr_t) &queryItem,
            .num_items = 1,
        };
        if (ioctl(fd, DRM_IOCTL_I915_QUERY, &query) >= 0 )
        {
            FF_AUTO_FREE uint8_t* buffer = calloc(1, (size_t) queryItem.length);
            queryItem.data_ptr = (uintptr_t) buffer;
            if (ioctl(fd, DRM_IOCTL_I915_QUERY, &query) >= 0)
            {
                gpu->dedicated.total = gpu->shared.total = gpu->dedicated.used = gpu->shared.used = 0;
                struct drm_i915_query_memory_regions* regionInfo = (void*) buffer;
                for (uint32_t i = 0; i < regionInfo->num_regions; i++)
                {
                    struct drm_i915_memory_region_info* region = regionInfo->regions + i;
                    switch (region->region.memory_class)
                    {
                    case I915_MEMORY_CLASS_SYSTEM:
                        gpu->shared.total += region->probed_size;
                        gpu->shared.used += region->probed_size - region->unallocated_size;
                        break;
                    case I915_MEMORY_CLASS_DEVICE:
                        gpu->dedicated.total += region->probed_size;
                        gpu->dedicated.used += region->probed_size - region->unallocated_size;
                        break;
                    }
                }
            }
        }
    }
    return NULL;
}

static const char* detectByDrm(const FFGPUOptions* options, FFlist* gpus)
{
    FF_LIBRARY_LOAD(libdrm, "dlopen libdrm" FF_LIBRARY_EXTENSION " failed", "libdrm" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, drmGetDevices)

    drmDevicePtr devices[64];
    int nDevices = ffdrmGetDevices(devices, ARRAY_SIZE(devices));

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

        FF_AUTO_CLOSE_FD int fd = open(path, O_RDONLY);
        if (fd < 0) continue;

        char driverName[64];
        char driverDesc[64];
        struct drm_version ver = {
            .name = driverName,
            .name_len = ARRAY_SIZE(driverName),
            .desc = driverDesc,
            .desc_len = ARRAY_SIZE(driverDesc),
        };
        if (ioctl(fd, DRM_IOCTL_VERSION, &ver) == 0)
            ffStrbufSetF(&gpu->driver, "%*s %d.%d.%d", (int) ver.name_len, ver.name, ver.version_major, ver.version_minor, ver.version_patchlevel);

        if (dev->bustype != DRM_BUS_PCI)
            continue;

        if (ffStrEquals(ver.name, "i915"))
            drmDetectIntelSpecific(gpu, fd);
        else if (ffStrEquals(ver.name, "amdgpu"))
            drmDetectAmdSpecific(options, gpu, dev->nodes[DRM_NODE_RENDER]);
        else if (ffStrEquals(ver.name, "nvidia-drm") && (options->temp || options->driverSpecific))
        {
            ffDetectNvidiaGpuInfo(&(FFGpuDriverCondition) {
                                      .type = FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID,
                                      .pciBusId = {
                                          .domain = (uint32_t) dev->businfo.pci->domain,
                                          .bus = dev->businfo.pci->bus,
                                          .device = dev->businfo.pci->dev,
                                          .func = dev->businfo.pci->func,
                                      },
                                  }, (FFGpuDriverResult) {
                                      .index = &gpu->index,
                                      .temp = options->temp ? &gpu->temperature : NULL,
                                      .memory = options->driverSpecific ? &gpu->dedicated : NULL,
                                      .coreCount = options->driverSpecific ? (uint32_t*) &gpu->coreCount : NULL,
                                      .type = &gpu->type,
                                      .frequency = &gpu->frequency,
                                      .coreUsage = &gpu->coreUsage,
                                      .name = &gpu->name,
                                  }, "libnvidia-ml.so");
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

    return NULL;
}
#endif

static const char* detectByPci(const FFGPUOptions* options, FFlist* gpus)
{
    FF_AUTO_CLOSE_FD int fd = open("/dev/pci", O_RDONLY, 0);
    if (fd < 0)
        return "open(\"/dev/pci\", O_RDONLY, 0) failed";

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

        if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_NVIDIA && (options->temp || options->driverSpecific))
        {
            ffDetectNvidiaGpuInfo(&(FFGpuDriverCondition) {
                                      .type = FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID,
                                      .pciBusId = {
                                          .domain = (uint32_t) pc->pc_sel.pc_domain,
                                          .bus = pc->pc_sel.pc_bus,
                                          .device = pc->pc_sel.pc_dev,
                                          .func = pc->pc_sel.pc_func,
                                      },
                                  }, (FFGpuDriverResult) {
                                      .index = &gpu->index,
                                      .temp = options->temp ? &gpu->temperature : NULL,
                                      .memory = options->driverSpecific ? &gpu->dedicated : NULL,
                                      .coreCount = options->driverSpecific ? (uint32_t*) &gpu->coreCount : NULL,
                                      .type = &gpu->type,
                                      .frequency = &gpu->frequency,
                                      .coreUsage = &gpu->coreUsage,
                                      .name = &gpu->name,
                                  }, "libnvidia-ml.so");
        }

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
    if (options->detectionMethod == FF_GPU_DETECTION_METHOD_AUTO)
    {
        detectByDrm(options, gpus);
        if (gpus->length > 0) return NULL;
    }

    return detectByPci(options, gpus);
}
