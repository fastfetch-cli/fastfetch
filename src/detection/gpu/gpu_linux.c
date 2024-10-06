#include "detection/gpu/gpu.h"
#include "detection/vulkan/vulkan.h"
#include "detection/cpu/cpu.h"
#include "detection/gpu/gpu_driver_specific.h"
#include "common/io/io.h"
#include "common/library.h"
#include "common/properties.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"

#include <inttypes.h>

#ifdef FF_HAVE_DRM_AMDGPU
    #include <amdgpu.h>
    #include <amdgpu_drm.h>
    #include <fcntl.h>
#endif

#ifdef FF_HAVE_DRM
    #include "intel_drm.h"
    #include <fcntl.h>
    #include <sys/ioctl.h>
#endif

#include "gpu_asahi.h"

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

static bool pciDetectDriver(FFstrbuf* result, FFstrbuf* pciDir, FFstrbuf* buffer, FF_MAYBE_UNUSED const char* drmKey)
{
    uint32_t pciDirLength = pciDir->length;
    ffStrbufAppendS(pciDir, "/driver");
    char pathBuf[PATH_MAX];
    ssize_t resultLength = readlink(pciDir->chars, pathBuf, sizeof(pathBuf));
    if(resultLength <= 0) return false;

    const char* slash = memrchr(pathBuf, '/', (size_t) resultLength);
    if (slash)
    {
        slash++;
        ffStrbufSetNS(result, (uint32_t) (resultLength - (slash - pathBuf)), slash);
    }

    if (ffStrbufEqualS(result, "nvidia"))
    {
        if (ffReadFileBuffer("/proc/driver/nvidia/version", buffer))
        {
            if (ffStrbufContainS(buffer, " Open "))
                ffStrbufAppendS(result, " (open source)");
            else
                ffStrbufAppendS(result, " (proprietary)");
        }
    }

    if (instance.config.general.detectVersion)
    {
        ffStrbufAppendS(pciDir, "/module/version");
        if (ffReadFileBuffer(pciDir->chars, buffer))
        {
            ffStrbufTrimRightSpace(buffer);
            ffStrbufAppendC(result, ' ');
            ffStrbufAppend(result, buffer);
        }
        else if (ffStrbufEqualS(result, "zx"))
        {
            ffStrbufSubstrBefore(pciDir, pciDirLength);
            ffStrbufAppendS(pciDir, "/zx_info/driver_version");
            if (ffReadFileBuffer(pciDir->chars, buffer))
            {
                ffStrbufTrimRightSpace(buffer);
                ffStrbufAppendC(result, ' ');
                ffStrbufAppend(result, buffer);
            }
        }
    }

    return true;
}

FF_MAYBE_UNUSED static const char* drmFindRenderFromCard(const char* drmCardKey, FFstrbuf* result)
{
    char path[PATH_MAX];
    sprintf(path, "/sys/class/drm/%s/device/drm", drmCardKey);
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(path);
    if (!dirp) return "Failed to open `/sys/class/drm/{drmCardKey}/device/drm`";

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (ffStrStartsWith(entry->d_name, "render"))
        {
            ffStrbufSetS(result, "/dev/dri/");
            ffStrbufAppendS(result, entry->d_name);
            return NULL;
        }
    }
    return "Failed to find render device";
}

static const char* drmDetectAmdSpecific(const FFGPUOptions* options, FFGPUResult* gpu, const char* drmKey, FFstrbuf* buffer)
{
    #if FF_HAVE_DRM_AMDGPU

    FF_LIBRARY_LOAD(libdrm, "dlopen libdrm_amdgpu" FF_LIBRARY_EXTENSION " failed", "libdrm_amdgpu" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_device_initialize)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_get_marketing_name)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_query_gpu_info)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_query_sensor_info)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_query_heap_info)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_device_deinitialize)

    {
        const char* error = drmFindRenderFromCard(drmKey, buffer);
        if (error) return error;
    }
    FF_AUTO_CLOSE_FD int fd = open(buffer->chars, O_RDONLY);
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

static void pciDetectAmdSpecific(const FFGPUOptions* options, FFGPUResult* gpu, FFstrbuf* pciDir, FFstrbuf* buffer)
{
    // https://www.kernel.org/doc/html/v5.10/gpu/amdgpu.html#mem-info-vis-vram-total
    const uint32_t pciDirLen = pciDir->length;

    ffStrbufAppendS(pciDir, "/hwmon/");
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(pciDir->chars);
    if (!dirp) return;

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_name[0] == '.') continue;
        break;
    }
    if (!entry) return;
    ffStrbufAppendS(pciDir, entry->d_name);
    ffStrbufAppendC(pciDir, '/');

    const uint32_t hwmonLen = pciDir->length;
    ffStrbufAppendS(pciDir, "in1_input"); // Northbridge voltage in millivolts (APUs only)
    if (ffPathExists(pciDir->chars, FF_PATHTYPE_ANY))
        gpu->type = FF_GPU_TYPE_INTEGRATED;
    else
        gpu->type = FF_GPU_TYPE_DISCRETE;

    uint64_t value = 0;
    if (options->temp)
    {
        ffStrbufSubstrBefore(pciDir, hwmonLen);
        ffStrbufAppendS(pciDir, "temp1_input"); // The on die GPU temperature in millidegrees Celsius
        if (ffReadFileBuffer(pciDir->chars, buffer) && (value = ffStrbufToUInt(buffer, 0)))
            gpu->temperature = (double) value / 1000;
    }

    if (options->driverSpecific)
    {
        ffStrbufSubstrBefore(pciDir, pciDirLen);
        ffStrbufAppendS(pciDir, "/mem_info_vis_vram_total");
        if (ffReadFileBuffer(pciDir->chars, buffer) && (value = ffStrbufToUInt(buffer, 0)))
        {
            if (gpu->type == FF_GPU_TYPE_DISCRETE)
                gpu->dedicated.total = value;
            else
                gpu->shared.total = value;

            ffStrbufSubstrBefore(pciDir, pciDir->length - (uint32_t) strlen("/mem_info_vis_vram_total"));
            ffStrbufAppendS(pciDir, "/mem_info_vis_vram_used");
            if (ffReadFileBuffer(pciDir->chars, buffer) && (value = ffStrbufToUInt(buffer, 0)))
            {
                if (gpu->type == FF_GPU_TYPE_DISCRETE)
                    gpu->dedicated.used = value;
                else
                    gpu->shared.used = value;
            }
        }
    }
}

static void pciDetectIntelSpecific(FFGPUResult* gpu, FFstrbuf* pciDir, FFstrbuf* buffer, const char* drmKey)
{
    // Works for Intel GPUs
    // https://patchwork.kernel.org/project/intel-gfx/patch/1422039866-11572-3-git-send-email-ville.syrjala@linux.intel.com/

    // 0000:00:02.0 is reserved for Intel integrated graphics
    gpu->type = gpu->deviceId == 20 ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;

    if (!drmKey) return;

    if (ffStrbufEqualS(&gpu->driver, "xe"))
        ffStrbufAppendS(pciDir, "/tile0/gt0/freq0/max_freq");
    else
        ffStrbufAppendF(pciDir, "/drm/%s/gt_max_freq_mhz", drmKey);
    if (ffReadFileBuffer(pciDir->chars, buffer))
        gpu->frequency = (uint32_t) ffStrbufToUInt(buffer, 0);
}

static inline int popcountBytes(uint8_t* bytes, uint32_t length)
{
    int count = 0;
    while (length >= 8)
    {
        count += __builtin_popcountll(*(uint64_t*) bytes);
        bytes += 8;
        length -= 8;
    }
    if (length >= 4)
    {
        count += __builtin_popcountl(*(uint32_t*) bytes);
        bytes += 4;
        length -= 4;
    }
    if (length >= 2)
    {
        count += __builtin_popcountl(*(uint16_t*) bytes);
        bytes += 2;
        length -= 2;
    }
    if (length)
    {
        count += __builtin_popcountl(*(uint8_t*) bytes);
    }
    return count;
}

static const char* drmDetectIntelSpecific(FFGPUResult* gpu, const char* drmKey, FFstrbuf* buffer)
{
    #if FF_HAVE_DRM
    ffStrbufSetS(buffer, "/dev/dri/");
    ffStrbufAppendS(buffer, drmKey);
    FF_AUTO_CLOSE_FD int fd = open(buffer->chars, O_RDONLY);
    if (fd < 0) return "Failed to open drm device";

    if (ffStrbufEqualS(&gpu->driver, "xe"))
    {
        {
            struct drm_xe_device_query query = {
                .extensions = 0,
                .query = DRM_XE_DEVICE_QUERY_GT_TOPOLOGY,
                .size = 0,
                .data = 0,
            };
            if (ioctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query) >= 0)
            {
                FF_AUTO_FREE uint8_t* buffer = malloc(query.size);
                query.data = (uintptr_t) buffer;
                if (ioctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query) >= 0)
                {
                    int dssCount = 0, euPerDssCount = 0;
                    for (struct drm_xe_query_topology_mask* topo = (void*) buffer;
                        (uint8_t*) topo < buffer + query.size;
                        topo = (void*) (topo->mask + topo->num_bytes)
                    ) {
                        switch (topo->type)
                        {
                            case DRM_XE_TOPO_DSS_COMPUTE:
                            case DRM_XE_TOPO_DSS_GEOMETRY:
                                dssCount += popcountBytes(topo->mask, topo->num_bytes);
                                break;
                            case DRM_XE_TOPO_EU_PER_DSS:
                                euPerDssCount += popcountBytes(topo->mask, topo->num_bytes);
                                break;
                        }
                    }
                    gpu->coreCount = dssCount * euPerDssCount;
                }
            }
        }

        {
            struct drm_xe_device_query query = {
                .query = DRM_XE_DEVICE_QUERY_MEM_REGIONS,
            };
            if (ioctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query) >= 0)
            {
                FF_AUTO_FREE uint8_t* buffer = malloc(query.size);
                query.data = (uintptr_t) buffer;
                if (ioctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query) >= 0)
                {
                    gpu->dedicated.total = gpu->shared.total = gpu->dedicated.used = gpu->shared.used = 0;
                    struct drm_xe_query_mem_regions* regionInfo = (void*) buffer;
                    for (uint32_t i = 0; i < regionInfo->num_mem_regions; i++)
                    {
                        struct drm_xe_mem_region* region = regionInfo->mem_regions + i;
                        switch (region->mem_class)
                        {
                            case DRM_XE_MEM_REGION_CLASS_SYSMEM:
                                gpu->shared.total += region->total_size;
                                gpu->shared.used += region->used;
                                break;
                            case DRM_XE_MEM_REGION_CLASS_VRAM:
                                gpu->dedicated.total += region->total_size;
                                gpu->dedicated.used += region->used;
                                break;
                        }
                    }
                }
            }
        }
    }
    else if (ffStrbufEqualS(&gpu->driver, "i915"))
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
    }
    return NULL;
    #else
    FF_UNUSED(gpu, drmKey, buffer);
    return "Fastfetch is not compiled with drm support";
    #endif
}

static const char* detectPci(const FFGPUOptions* options, FFlist* gpus, FFstrbuf* buffer, FFstrbuf* deviceDir, const char* drmKey)
{
    const uint32_t drmDirPathLength = deviceDir->length;
    uint32_t vendorId, deviceId, subVendorId, subDeviceId;
    uint8_t classId, subclassId;
    if (sscanf(buffer->chars + strlen("pci:"), "v%8" SCNx32 "d%8" SCNx32 "sv%8" SCNx32 "sd%8" SCNx32 "bc%2" SCNx8 "sc%2" SCNx8, &vendorId, &deviceId, &subVendorId, &subDeviceId, &classId, &subclassId) != 6)
        return "Failed to parse pci modalias";

    if (classId != 0x03 /*PCI_BASE_CLASS_DISPLAY*/)
        return "Not a GPU device";

    char pciPath[PATH_MAX];
    const char* pPciPath = NULL;
    if (drmKey)
    {
        ssize_t pathLength = readlink(deviceDir->chars, pciPath, sizeof(pciPath) - 1);
        if (pathLength <= 0)
            return "Unable to get PCI device path";
        pciPath[pathLength] = '\0';
        pPciPath = strrchr(pciPath, '/');
        if (__builtin_expect(pPciPath != NULL, true))
            pPciPath++;
        else
            pPciPath = pciPath;
    }
    else
    {
        pPciPath = memrchr(deviceDir->chars, '/', deviceDir->length) + 1;
    }

    uint32_t pciDomain, pciBus, pciDevice, pciFunc;
    if (sscanf(pPciPath, "%" SCNx32 ":%" SCNx32 ":%" SCNx32 ".%" SCNx32, &pciDomain, &pciBus, &pciDevice, &pciFunc) != 4)
        return "Invalid PCI device path";

    FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
    ffStrbufInitStatic(&gpu->vendor, ffGetGPUVendorString((uint16_t) vendorId));
    ffStrbufInit(&gpu->name);
    ffStrbufInit(&gpu->driver);
    ffStrbufInit(&gpu->platformApi);
    gpu->index = FF_GPU_INDEX_UNSET;
    gpu->temperature = FF_GPU_TEMP_UNSET;
    gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
    gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
    gpu->type = FF_GPU_TYPE_UNKNOWN;
    gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
    gpu->deviceId = (pciDomain * 100000ull) + (pciBus * 1000ull) + (pciDevice * 10ull) + pciFunc;
    gpu->frequency = FF_GPU_FREQUENCY_UNSET;

    char drmKeyBuffer[8];
    if (!drmKey)
    {
        ffStrbufAppendS(deviceDir, "/drm");
        FF_AUTO_CLOSE_DIR DIR* dirp = opendir(deviceDir->chars);
        if (dirp)
        {
            struct dirent* entry;
            while ((entry = readdir(dirp)) != NULL)
            {
                if (ffStrStartsWith(entry->d_name, "card"))
                {
                    strncpy(drmKeyBuffer, entry->d_name, sizeof(drmKeyBuffer) - 1);
                    drmKeyBuffer[sizeof(drmKeyBuffer) - 1] = '\0';
                    drmKey = drmKeyBuffer;
                    break;
                }
            }
        }
        ffStrbufSubstrBefore(deviceDir, drmDirPathLength);
    }

    if (drmKey) ffStrbufSetF(&gpu->platformApi, "DRM (%s)", drmKey);

    pciDetectDriver(&gpu->driver, deviceDir, buffer, drmKey);
    ffStrbufSubstrBefore(deviceDir, drmDirPathLength);

    if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
    {
        bool ok = false;
        if (drmKey && options->driverSpecific)
            ok = drmDetectAmdSpecific(options, gpu, drmKey, buffer) == NULL;

        if (!ok)
        {
            pciDetectAmdSpecific(options, gpu, deviceDir, buffer);
            ffStrbufSubstrBefore(deviceDir, drmDirPathLength);

            ffStrbufAppendS(deviceDir, "/revision");
            if (ffReadFileBuffer(deviceDir->chars, buffer))
            {
                char* pend;
                uint64_t revision = strtoul(buffer->chars, &pend, 16);
                if (pend != buffer->chars)
                {
                    char query[32];
                    snprintf(query, sizeof(query), "%X,\t%X,", (unsigned) deviceId, (unsigned) revision);
                    #ifdef FF_CUSTOM_AMDGPU_IDS_PATH
                    ffParsePropFile(FF_STR(FF_CUSTOM_AMDGPU_IDS_PATH), query, &gpu->name);
                    #else
                    ffParsePropFileData("libdrm/amdgpu.ids", query, &gpu->name);
                    #endif
                }
            }
            ffStrbufSubstrBefore(deviceDir, drmDirPathLength);
        }
    }
    else if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_INTEL)
    {
        pciDetectIntelSpecific(gpu, deviceDir, buffer, drmKey);
        ffStrbufSubstrBefore(deviceDir, drmDirPathLength);
        if (options->driverSpecific && drmKey)
            drmDetectIntelSpecific(gpu, drmKey, buffer);
    }
    else
    {
        __typeof__(&ffDetectNvidiaGpuInfo) detectFn;
        const char* soName;
        if (getDriverSpecificDetectionFn(gpu->vendor.chars, &detectFn, &soName) && (options->temp || options->driverSpecific))
        {
            detectFn(&(FFGpuDriverCondition) {
                .type = FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID,
                .pciBusId = {
                    .domain = pciDomain,
                    .bus = pciBus,
                    .device = pciDevice,
                    .func = pciFunc,
                },
            }, (FFGpuDriverResult) {
                .index = &gpu->index,
                .temp = options->temp ? &gpu->temperature : NULL,
                .memory = options->driverSpecific ? &gpu->dedicated : NULL,
                .coreCount = options->driverSpecific ? (uint32_t*) &gpu->coreCount : NULL,
                .coreUsage = options->driverSpecific ? &gpu->coreUsage : NULL,
                .type = &gpu->type,
                .frequency = options->driverSpecific ? &gpu->frequency : NULL,
                .name = &gpu->name,
            }, soName);
        }

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
        }
    }

    if (gpu->name.length == 0)
        ffGPUFillVendorAndName(subclassId, (uint16_t) vendorId, (uint16_t) deviceId, gpu);

    return NULL;
}

#if __aarch64__

FF_MAYBE_UNUSED static const char* drmDetectAsahiSpecific(FFGPUResult* gpu, const char* name, FFstrbuf* buffer, const char* drmKey)
{
    if (sscanf(name, "agx-t%lu", &gpu->deviceId) == 1)
        ffStrbufSetStatic(&gpu->name, ffCPUAppleCodeToName((uint32_t) gpu->deviceId));
    ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_APPLE);

    #if FF_HAVE_DRM
    ffStrbufSetS(buffer, "/dev/dri/");
    ffStrbufAppendS(buffer, drmKey);
    FF_AUTO_CLOSE_FD int fd = open(buffer->chars, O_RDONLY);
    if (fd >= 0)
    {
        struct drm_asahi_params_global paramsGlobal = {};
        if (ioctl(fd, DRM_IOCTL_ASAHI_GET_PARAMS, &(struct drm_asahi_get_params) {
            .param_group = DRM_ASAHI_GET_PARAMS,
            .pointer = (uint64_t) &paramsGlobal,
            .size = sizeof(paramsGlobal),
        }) >= 0)
        {
            ffStrbufAppendF(&gpu->driver, " %u", paramsGlobal.unstable_uabi_version);

            // FIXME: They will introduce ABI breaking changes. Always check the latest version
            // https://www.reddit.com/r/AsahiLinux/comments/1ei2qiv/comment/lgm0v5s/
            if (paramsGlobal.unstable_uabi_version == DRM_ASAHI_UNSTABLE_UABI_VERSION)
            {
                gpu->coreCount = (int) paramsGlobal.num_cores_total_active;
                gpu->frequency = paramsGlobal.max_frequency_khz / 1000;
                gpu->deviceId = paramsGlobal.chip_id;

                if (!gpu->name.length)
                {
                    const char* variant = " Unknown";
                    switch (paramsGlobal.gpu_variant) {
                    case 'G':
                        variant = "";
                        break;
                    case 'S':
                        variant = " Pro";
                        break;
                    case 'C':
                        variant = " Max";
                        break;
                    case 'D':
                        variant = " Ultra";
                        break;
                    }
                    ffStrbufSetF(&gpu->name, "Apple M%d%s (G%d%c %02X)",
                        paramsGlobal.gpu_generation - 12, variant,
                        paramsGlobal.gpu_generation, paramsGlobal.gpu_variant,
                        paramsGlobal.gpu_revision + 0xA0);
                }
            }
        }
    }
    #endif

    return NULL;
}
#endif

static const char* detectOf(FFlist* gpus, FFstrbuf* buffer, FFstrbuf* drmDir, const char* drmKey)
{
    char compatible[256]; // vendor,model-name
    if (sscanf(buffer->chars + strlen("of:"), "NgpuT%*[^C]C%255[^C]", compatible) != 1)
        return "Failed to parse of modalias or not a GPU device";

    char* name = strchr(compatible, ',');
    if (name)
    {
        *name = '\0';
        ++name;
    }

    FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
    gpu->index = FF_GPU_INDEX_UNSET;
    gpu->deviceId = 0;
    ffStrbufInit(&gpu->name);
    ffStrbufInit(&gpu->vendor);
    ffStrbufInit(&gpu->driver);
    ffStrbufInitF(&gpu->platformApi, "DRM (%s)", drmKey);
    gpu->temperature = FF_GPU_TEMP_UNSET;
    gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
    gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
    gpu->type = FF_GPU_TYPE_INTEGRATED;
    gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
    gpu->frequency = FF_GPU_FREQUENCY_UNSET;

    pciDetectDriver(&gpu->driver, drmDir, buffer, drmKey);

    #ifdef __aarch64__
    if (ffStrbufEqualS(&gpu->driver, "asahi"))
        drmDetectAsahiSpecific(gpu, name, buffer, drmKey);
    #endif

    if (!gpu->name.length)
    {
        ffStrbufSetS(&gpu->name, name ? name : compatible);
        ffStrbufTrimRightSpace(&gpu->name);
    }
    if (!gpu->vendor.length && name)
    {
        if (ffStrEquals(compatible, "brcm"))
            ffStrbufSetStatic(&gpu->vendor, "Broadcom"); // Raspberry Pi
        else
        {
            ffStrbufSetS(&gpu->vendor, compatible);
            gpu->vendor.chars[0] = (char) toupper(compatible[0]);
        }
    }

    return NULL;
}

static const char* drmDetectGPUs(const FFGPUOptions* options, FFlist* gpus)
{
    FF_STRBUF_AUTO_DESTROY drmDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&drmDir, "/sys/class/drm/");
    const uint32_t drmDirLength = drmDir.length;

    FF_AUTO_CLOSE_DIR DIR* dir = opendir(drmDir.chars);
    if(dir == NULL)
        return "Failed to open `/sys/class/drm/`";

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (!ffStrStartsWith(entry->d_name, "card") ||
            strchr(entry->d_name + 4, '-') != NULL)
            continue;

        ffStrbufAppendS(&drmDir, entry->d_name);

        ffStrbufAppendS(&drmDir, "/device/modalias");
        if (!ffReadFileBuffer(drmDir.chars, &buffer))
            continue;
        ffStrbufSubstrBefore(&drmDir, drmDir.length - (uint32_t) strlen("/modalias"));

        if (ffStrbufStartsWithS(&buffer, "pci:"))
            detectPci(options, gpus, &buffer, &drmDir, entry->d_name);
        else if (ffStrbufStartsWithS(&buffer, "of:"))
            detectOf(gpus, &buffer, &drmDir, entry->d_name);

        ffStrbufSubstrBefore(&drmDir, drmDirLength);
    }

    return NULL;
}


static const char* pciDetectGPUs(const FFGPUOptions* options, FFlist* gpus)
{
    //https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-bus-pci
    const char* pciDirPath = "/sys/bus/pci/devices/";

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(pciDirPath);
    if(dirp == NULL)
        return "Failed to open `/sys/bus/pci/devices/`";

    FF_STRBUF_AUTO_DESTROY pciDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&pciDir, pciDirPath);

    const uint32_t pciBaseDirLength = pciDir.length;

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(entry->d_name[0] == '.')
            continue;

        ffStrbufSubstrBefore(&pciDir, pciBaseDirLength);
        ffStrbufAppendS(&pciDir, entry->d_name);
        const uint32_t pciDevDirLength = pciDir.length;

        ffStrbufAppendS(&pciDir, "/modalias");
        if (!ffReadFileBuffer(pciDir.chars, &buffer))
            continue;
        ffStrbufSubstrBefore(&pciDir, pciDevDirLength);
        assert(ffStrbufStartsWithS(&buffer, "pci:"));

        detectPci(options, gpus, &buffer, &pciDir, NULL);
        ffStrbufSubstrBefore(&pciDir, pciBaseDirLength);
    }

    return NULL;
}

const char* ffDetectGPUImpl(const FFGPUOptions* options, FFlist* gpus)
{
    #ifdef FF_HAVE_DIRECTX_HEADERS
        const char* ffGPUDetectByDirectX(const FFGPUOptions* options, FFlist* gpus);
        if (ffGPUDetectByDirectX(options, gpus) == NULL)
            return NULL;
    #endif

    if (options->detectionMethod == FF_GPU_DETECTION_METHOD_AUTO)
    {
        if (drmDetectGPUs(options, gpus) == NULL && gpus->length > 0)
            return NULL;
    }
    return pciDetectGPUs(options, gpus);
}
