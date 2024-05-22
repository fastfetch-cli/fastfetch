#include "detection/gpu/gpu.h"
#include "detection/vulkan/vulkan.h"
#include "detection/temps/temps_linux.h"
#include "detection/cpu/cpu.h"
#include "common/io/io.h"
#include "common/properties.h"
#include "util/stringUtils.h"

#ifdef FF_USE_PROPRIETARY_GPU_DRIVER_API
    #include "detection/gpu/gpu_driver_specific.h"
#endif

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

#include <inttypes.h>

#if __has_include(<drm/drm.h>)
    #include <drm/drm.h>
    #define FF_HAVE_DRM_H 1
#elif __has_include(<libdrm/drm.h>)
    #include <libdrm/drm.h>
    #define FF_HAVE_DRM_H 1
#endif

#if FF_HAVE_DRM_H
    #include <fcntl.h>
    #include <sys/ioctl.h>

    #if __aarch64__ && __has_include(<drm/asahi_drm.h>)
        #include <drm/asahi_drm.h>
        #define FF_HAVE_ASAHI_DRM_H 1
    #endif
#endif

static bool pciDetectDriver(FFGPUResult* gpu, FFstrbuf* pciDir, FFstrbuf* buffer, FF_MAYBE_UNUSED const char* drmKey)
{
    ffStrbufAppendS(pciDir, "/driver");
    char pathBuf[PATH_MAX];
    ssize_t resultLength = readlink(pciDir->chars, pathBuf, sizeof(pathBuf));
    if(resultLength > 0)
    {
        const char* slash = memrchr(pathBuf, '/', (size_t) resultLength);
        if (slash)
        {
            slash++;
            ffStrbufSetNS(&gpu->driver, (uint32_t) (resultLength - (slash - pathBuf)), slash);
        }

        ffStrbufAppendS(pciDir, "/module/version");
        if (ffReadFileBuffer(pciDir->chars, buffer))
        {
            ffStrbufTrimRightSpace(buffer);
            ffStrbufAppendC(&gpu->driver, ' ');
            ffStrbufAppend(&gpu->driver, buffer);
            return true;
        }
    }

    return false;
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
    if (ffPathExists(pciDir->chars, FF_PATHTYPE_FILE))
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

    ffStrbufSubstrBefore(pciDir, hwmonLen);
    ffStrbufAppendS(pciDir, "freq1_input"); // The gfx/compute clock in hertz
    if (ffReadFileBuffer(pciDir->chars, buffer) && (value = ffStrbufToUInt(buffer, 0)))
        gpu->frequency = (double) value / (1000 * 1000 * 1000);

    if (options->driverSpecific)
    {
        ffStrbufSubstrBefore(pciDir, pciDirLen);
        ffStrbufAppendS(pciDir, "/mem_info_vis_vram_total");
        if (ffReadFileBuffer(pciDir->chars, buffer) && (value = ffStrbufToUInt(buffer, 0)))
        {
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

static void pciDetectIntelSpecific(FFGPUResult* gpu, FFstrbuf* pciDir, FFstrbuf* buffer)
{
    // Works for Intel GPUs
    // https://patchwork.kernel.org/project/intel-gfx/patch/1422039866-11572-3-git-send-email-ville.syrjala@linux.intel.com/

    if (ffStrbufStartsWithS(&gpu->name, "Intel "))
        ffStrbufSubstrAfter(&gpu->name, (uint32_t) strlen("Intel "));
    gpu->type = ffStrbufStartsWithIgnCaseS(&gpu->name, "Arc ") ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;

    ffStrbufAppendS(pciDir, "/drm/");
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(pciDir->chars);
    if (!dirp) return;
    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (ffStrStartsWith(entry->d_name, "card")) break;
    }
    if (!entry) return;
    ffStrbufAppendS(pciDir, entry->d_name);
    ffStrbufAppendS(pciDir, "/gt_max_freq_mhz");
    if (ffReadFileBuffer(pciDir->chars, buffer))
        gpu->frequency = ffStrbufToDouble(buffer) / 1000.0;
}

static bool loadPciIds(FFstrbuf* pciids)
{
    #ifdef FF_CUSTOM_PCI_IDS_PATH

    ffReadFileBuffer(FF_STR(FF_CUSTOM_PCI_IDS_PATH), pciids);
    if (pciids->length > 0) return true;

    #else

    ffReadFileBuffer(FASTFETCH_TARGET_DIR_USR "/share/hwdata/pci.ids", pciids);
    if (pciids->length > 0) return true;

    ffReadFileBuffer(FASTFETCH_TARGET_DIR_USR "/share/misc/pci.ids", pciids); // debian?
    if (pciids->length > 0) return true;

    ffReadFileBuffer(FASTFETCH_TARGET_DIR_USR "/local/share/hwdata/pci.ids", pciids);
    if (pciids->length > 0) return true;

    #endif

    return false;
}

static const char* detectPci(const FFGPUOptions* options, FFlist* gpus, FFstrbuf* buffer, FFstrbuf* deviceDir, const char* drmKey)
{
    const uint32_t drmDirPathLength = deviceDir->length;
    uint32_t vendorId, deviceId, subVendorId, subDeviceId;
    uint8_t classId, subclassId;
    if (sscanf(buffer->chars + strlen("pci:"), "v%8" SCNx32 "d%8" SCNx32 "sv%8" SCNx32 "sd%8" SCNx32 "bc%2" SCNx8 "sc%2" SCNx8, &vendorId, &deviceId, &subVendorId, &subDeviceId, &classId, &subclassId) != 6)
        return "Invalid modalias string";

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
    gpu->temperature = FF_GPU_TEMP_UNSET;
    gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
    gpu->type = FF_GPU_TYPE_UNKNOWN;
    gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
    gpu->deviceId = ((uint64_t) pciDomain << 6) | ((uint64_t) pciBus << 4) | (deviceId << 2) | pciFunc;
    gpu->frequency = FF_GPU_FREQUENCY_UNSET;

    if (drmKey) ffStrbufSetF(&gpu->platformApi, "DRM (%s)", drmKey);

    if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
    {
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

    if (gpu->name.length == 0)
    {
        static FFstrbuf pciids;
        if (pciids.chars == NULL)
        {
            ffStrbufInit(&pciids);
            loadPciIds(&pciids);
        }
        ffGPUParsePciIds(&pciids, subclassId, (uint16_t) vendorId, (uint16_t) deviceId, gpu);
    }

    pciDetectDriver(gpu, deviceDir, buffer, drmKey);
    ffStrbufSubstrBefore(deviceDir, drmDirPathLength);

    if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
    {
        pciDetectAmdSpecific(options, gpu, deviceDir, buffer);
        ffStrbufSubstrBefore(deviceDir, drmDirPathLength);
    }
    else if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_INTEL)
    {
        pciDetectIntelSpecific(gpu, deviceDir, buffer);
        ffStrbufSubstrBefore(deviceDir, drmDirPathLength);
    }
    else if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_NVIDIA)
    {
        #ifdef FF_USE_PROPRIETARY_GPU_DRIVER_API
        if (options->temp || options->driverSpecific)
        {
            ffDetectNvidiaGpuInfo(&(FFGpuDriverCondition) {
                .type = FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID,
                .pciBusId = {
                    .domain = pciDomain,
                    .bus = pciBus,
                    .device = pciDevice,
                    .func = pciFunc,
                },
            }, (FFGpuDriverResult) {
                .temp = options->temp ? &gpu->temperature : NULL,
                .memory = options->driverSpecific ? &gpu->dedicated : NULL,
                .coreCount = options->driverSpecific ? (uint32_t*) &gpu->coreCount : NULL,
                .type = &gpu->type,
                .frequency = &gpu->frequency,
            }, "libnvidia-ml.so");
        }
        #endif // FF_USE_PROPRIETARY_GPU_DRIVER_API

        if (gpu->type == FF_GPU_TYPE_UNKNOWN)
        {
            if (ffStrbufStartsWithIgnCaseS(&gpu->name, "GeForce") ||
                ffStrbufStartsWithIgnCaseS(&gpu->name, "Quadro") ||
                ffStrbufStartsWithIgnCaseS(&gpu->name, "Tesla"))
                gpu->type = FF_GPU_TYPE_DISCRETE;
        }
    }

    return NULL;
}

FF_MAYBE_UNUSED static const char* detectAsahi(FFlist* gpus, FFstrbuf* buffer, FFstrbuf* drmDir, const char* drmKey)
{
    uint32_t index = ffStrbufFirstIndexS(buffer, "apple,agx-t");
    if (index == buffer->length) return "display-subsystem?";
    index += (uint32_t) strlen("apple,agx-t");

    FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
    gpu->deviceId = strtoul(buffer->chars + index, NULL, 10);
    ffStrbufInitStatic(&gpu->name, ffCPUAppleCodeToName((uint32_t) gpu->deviceId));
    ffStrbufInitStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_APPLE);
    ffStrbufInit(&gpu->driver);
    ffStrbufInitF(&gpu->platformApi, "DRM (%s)", drmKey);
    gpu->temperature = FF_GPU_TEMP_UNSET;
    gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
    gpu->type = FF_GPU_TYPE_INTEGRATED;
    gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
    gpu->frequency = FF_GPU_FREQUENCY_UNSET;

    pciDetectDriver(gpu, drmDir, buffer, drmKey);

    #if FF_HAVE_ASAHI_DRM_H
    ffStrbufSetS(buffer, "/dev/dri/");
    ffStrbufAppendS(buffer, drmKey);
    FF_AUTO_CLOSE_FD int fd = open(buffer->chars, O_RDONLY);
    if (fd >= 0)
    {
        struct drm_asahi_params_global paramsGlobal = {};
        if (ioctl(fd, DRM_IOCTL_ASAHI_GET_PARAMS, &(struct drm_asahi_get_params){
            .param_group = DRM_ASAHI_GET_PARAMS,
            .pointer = (uint64_t) &paramsGlobal,
            .size = sizeof(paramsGlobal),
        }) >= 0)
        {
            gpu->coreCount = (int) paramsGlobal.num_cores_total_active;
            gpu->frequency = paramsGlobal.max_frequency_khz / 1e6;
            gpu->deviceId = paramsGlobal.chip_id;
        }
    }
    #endif

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
        #ifdef __aarch64__
        else if (ffStrbufStartsWithS(&buffer, "of:"))
            detectAsahi(gpus, &buffer, &drmDir, entry->d_name);
        #endif

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
    if (options->detectionMethod == FF_GPU_DETECTION_METHOD_AUTO)
    {
        #ifdef FF_HAVE_DIRECTX_HEADERS
            const char* ffGPUDetectByDirectX(const FFGPUOptions* options, FFlist* gpus);
            if (ffGPUDetectByDirectX(options, gpus) == NULL)
                return NULL;
        #endif

        if (drmDetectGPUs(options, gpus) == NULL && gpus->length > 0)
            return NULL;
    }
    return pciDetectGPUs(options, gpus);
}
