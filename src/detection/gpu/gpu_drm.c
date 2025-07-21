#include "gpu.h"

#if FF_HAVE_DRM
#include <drm.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "common/io/io.h"
#include "common/library.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"

#include "intel_drm.h"
#include "asahi_drm.h"
#include <radeon_drm.h>
#include <nouveau_drm.h>

const char* ffDrmDetectRadeon(const FFGPUOptions* options, FFGPUResult* gpu, const char* renderPath)
{
    FF_AUTO_CLOSE_FD int fd = open(renderPath, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return "Failed to open DRM render device";

    uint32_t value;

    // https://github.com/torvalds/linux/blob/fb4d33ab452ea254e2c319bac5703d1b56d895bf/drivers/gpu/drm/radeon/radeon_kms.c#L231

    if (ioctl(fd, DRM_IOCTL_RADEON_INFO, &(struct drm_radeon_info) {
        .request = RADEON_INFO_ACTIVE_CU_COUNT,
        .value = (uintptr_t) &value,
    }) >= 0)
        gpu->coreCount = (int32_t) value;

    if (options->temp)
    {
        if (ioctl(fd, DRM_IOCTL_RADEON_INFO, &(struct drm_radeon_info) {
            .request = RADEON_INFO_CURRENT_GPU_TEMP, // millidegrees C
            .value = (uintptr_t) &value,
        }) >= 0 && value != 0) // 0 means unavailable
            gpu->temperature = (double) value / 1000.0;
    }

    if (ioctl(fd, DRM_IOCTL_RADEON_INFO, &(struct drm_radeon_info) {
        .request = RADEON_INFO_MAX_SCLK, // MHz
        .value = (uintptr_t) &value,
    }) >= 0)
        gpu->frequency = (uint32_t) (value / 1000u);

    if (options->driverSpecific)
    {
        struct drm_radeon_gem_info gemInfo;
        if (ioctl(fd, DRM_IOCTL_RADEON_GEM_INFO, &gemInfo) >= 0)
        {
            // vram_usage can be bigger than vram_usage, so we use vram_size here
            gpu->dedicated.total = gemInfo.vram_size;
            gpu->shared.total = gemInfo.gart_size;

            uint64_t memSize;
            if (ioctl(fd, DRM_IOCTL_RADEON_INFO, &(struct drm_radeon_info) {
                .request = RADEON_INFO_VRAM_USAGE, // uint64_t
                .value = (uintptr_t) &memSize,
            }) >= 0)
                gpu->dedicated.used = memSize;

            if (ioctl(fd, DRM_IOCTL_RADEON_INFO, &(struct drm_radeon_info) {
                .request = RADEON_INFO_GTT_USAGE, // uint64_t
                .value = (uintptr_t) &memSize,
            }) >= 0)
                gpu->shared.used = memSize;
        }
    }

    return NULL;
}

#ifdef FF_HAVE_DRM_AMDGPU
#include <amdgpu.h>
#include <amdgpu_drm.h>

const char* ffDrmDetectAmdgpu(const FFGPUOptions* options, FFGPUResult* gpu, const char* renderPath)
{
#if FF_HAVE_DRM_AMDGPU
    FF_LIBRARY_LOAD(libdrm, "dlopen libdrm_amdgpu" FF_LIBRARY_EXTENSION " failed", "libdrm_amdgpu" FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_device_initialize)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_get_marketing_name)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_query_gpu_info)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_query_sensor_info)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_query_heap_info)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libdrm, amdgpu_device_deinitialize)

    FF_AUTO_CLOSE_FD int fd = open(renderPath, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return "Failed to open DRM render device";

    amdgpu_device_handle handle;
    uint32_t majorVersion, minorVersion;
    if (ffamdgpu_device_initialize(fd, &majorVersion, &minorVersion, &handle) < 0)
        return "Failed to initialize AMDGPU device";

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
            gpu->dedicated.total = heapInfo.heap_size;
            gpu->dedicated.used = heapInfo.heap_usage;
        }
        if (ffamdgpu_query_heap_info(handle, AMDGPU_GEM_DOMAIN_GTT, 0, &heapInfo) >= 0)
        {
            gpu->shared.total = heapInfo.heap_size;
            gpu->shared.used = heapInfo.heap_usage;
        }
    }

    if (ffamdgpu_query_sensor_info(handle, AMDGPU_INFO_SENSOR_GPU_LOAD, sizeof(value), &value) >= 0)
        gpu->coreUsage = value;

    ffamdgpu_device_deinitialize(handle);

    return NULL;
#else
    FF_UNUSED(gpu, renderPath);
    return "Fastfetch is compiled without libdrm support";
#endif
}
#endif

const char* ffDrmDetectI915(FFGPUResult* gpu, int fd)
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

const char* ffDrmDetectXe(FFGPUResult* gpu, int fd)
{
    bool flag = false;
    {
        struct drm_xe_device_query query = {
            .query = DRM_XE_DEVICE_QUERY_GT_TOPOLOGY,
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
                flag = true;
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
                flag = true;
            }
        }
    }
    return flag ? NULL : "Failed to query Xe GPU information";
}

const char* ffDrmDetectAsahi(FFGPUResult* gpu, int fd)
{
    struct drm_asahi_params_global paramsGlobal = {};
    if (ioctl(fd, DRM_IOCTL_ASAHI_GET_PARAMS, &(struct drm_asahi_get_params) {
        .param_group = DRM_ASAHI_GET_PARAMS,
        .pointer = (uintptr_t) &paramsGlobal,
        .size = sizeof(paramsGlobal),
    }) >= 0)
    {
        // They removed `unstable_uabi_version` from the struct. Hopefully they won't introduce new ABI changes.
        gpu->coreCount = (int32_t) (paramsGlobal.num_clusters_total * paramsGlobal.num_cores_per_cluster);
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

        return NULL;
    }

    return "Failed to query Asahi GPU information";
}

#ifndef DRM_IOCTL_NOUVEAU_GETPARAM
#define DRM_IOCTL_NOUVEAU_GETPARAM DRM_IOWR(DRM_COMMAND_BASE + DRM_NOUVEAU_GETPARAM, struct drm_nouveau_getparam)
#endif

const char* ffDrmDetectNouveau(FFGPUResult* gpu, int fd)
{
    struct drm_nouveau_getparam getparam = { };

    getparam.param = NOUVEAU_GETPARAM_FB_SIZE;
    if (ioctl(fd, DRM_IOCTL_NOUVEAU_GETPARAM, &getparam) == 0)
        gpu->dedicated.total = getparam.value;

    getparam.param = NOUVEAU_GETPARAM_AGP_SIZE;
    if (ioctl(fd, DRM_IOCTL_NOUVEAU_GETPARAM, &getparam) == 0)
        gpu->shared.total = getparam.value;

    getparam.param = NOUVEAU_GETPARAM_GRAPH_UNITS;
    if (ioctl(fd, DRM_IOCTL_NOUVEAU_GETPARAM, &getparam) == 0 && getparam.value < INT32_MAX)
        gpu->coreCount = (int32_t) getparam.value;

    return NULL;
}

#endif // FF_HAVE_DRM

#include "gpu_driver_specific.h"

const char* ffGPUDetectDriverSpecific(const FFGPUOptions* options, FFGPUResult* gpu, FFGpuDriverPciBusId pciBusId)
{
    __typeof__(&ffDetectNvidiaGpuInfo) detectFn;
    const char* soName;
    if (getDriverSpecificDetectionFn(gpu->vendor.chars, &detectFn, &soName) && (options->temp || options->driverSpecific))
    {
        return detectFn(&(FFGpuDriverCondition) {
            .type = FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID,
            .pciBusId = pciBusId,
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

    return "No driver-specific detection function found for the GPU vendor";
}
