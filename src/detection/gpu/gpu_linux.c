#include "detection/gpu/gpu.h"
#include "detection/vulkan/vulkan.h"
#include "detection/cpu/cpu.h"
#include "detection/gpu/gpu_driver_specific.h"
#include "common/io/io.h"
#include "common/library.h"
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

#if defined(FF_HAVE_DRM) && defined(__aarch64__)
    // https://github.com/alyssarosenzweig/linux/blob/agx-uapi-v7/include/uapi/drm/asahi_drm.h
    // Found in kernel-headers-6.14.4-400.asahi.fc42.aarch64
    #if __has_include(<drm/asahi_drm.h>)
        #include <drm/asahi_drm.h>
    #else
        #include "asahi_drm.h"
    #endif
    #define FF_HAVE_DRM_ASAHI 1
#endif

static bool pciDetectDriver(FFstrbuf* result, FFstrbuf* pciDir, FFstrbuf* buffer, FF_MAYBE_UNUSED const char* drmKey)
{
    uint32_t pciDirLength = pciDir->length;
    ffStrbufAppendS(pciDir, "/driver");
    char pathBuf[PATH_MAX];
    ssize_t resultLength = readlink(pciDir->chars, pathBuf, ARRAY_SIZE(pathBuf));
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
    #if FF_HAVE_DRM
    const char* error = drmFindRenderFromCard(drmKey, buffer);
    if (error) return error;
    if (ffStrbufEqualS(&gpu->driver, "radeon"))
        return ffDrmDetectRadeon(options, gpu, buffer->chars);
    else
    {
        #if FF_HAVE_DRM_AMDGPU
        return ffDrmDetectAmdgpu(options, gpu, buffer->chars);
        #else
        FF_UNUSED(options, gpu, drmKey, buffer);
        return "Fastfetch is not compiled with libdrm_amdgpu support";
        #endif
    }
    #else
    FF_UNUSED(gpu, drmKey, buffer);
    return "Fastfetch is not compiled with drm support";
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
    uint64_t value = 0;
    if (options->temp)
    {
        ffStrbufAppendS(pciDir, "temp1_input"); // The on die GPU temperature in millidegrees Celsius
        if (ffReadFileBuffer(pciDir->chars, buffer) && (value = ffStrbufToUInt(buffer, 0)))
            gpu->temperature = (double) value / 1000;
    }

    if (ffStrbufEqualS(&gpu->driver, "amdgpu")) // Ancient radeon drivers don't have these files
    {
        ffStrbufSubstrBefore(pciDir, hwmonLen);
        ffStrbufAppendS(pciDir, "in1_input"); // Northbridge voltage in millivolts (APUs only)
        if (ffPathExists(pciDir->chars, FF_PATHTYPE_ANY))
            gpu->type = FF_GPU_TYPE_INTEGRATED;
        else
            gpu->type = FF_GPU_TYPE_DISCRETE;

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

            ffStrbufSubstrBefore(pciDir, pciDirLen);
            ffStrbufAppendS(pciDir, "/gpu_busy_percent");
            if (ffReadFileBuffer(pciDir->chars, buffer) && (value = ffStrbufToUInt(buffer, 0)))
                gpu->coreUsage = (double) value;
        }
    }
}

static void pciDetectIntelSpecific(const FFGPUOptions* options, FFGPUResult* gpu, FFstrbuf* pciDir, FFstrbuf* buffer, const char* drmKey)
{
    // Works for Intel GPUs
    // https://patchwork.kernel.org/project/intel-gfx/patch/1422039866-11572-3-git-send-email-ville.syrjala@linux.intel.com/

    // 0000:00:02.0 is reserved for Intel integrated graphics
    gpu->type = gpu->deviceId == 20 ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;

    if (!drmKey) return;

    const uint32_t pciDirLen = pciDir->length;

    bool isXE = ffStrbufEqualS(&gpu->driver, "xe");
    if (isXE)
        ffStrbufAppendS(pciDir, "/tile0/gt0/freq0/max_freq");
    else
        ffStrbufAppendF(pciDir, "/drm/%s/gt_max_freq_mhz", drmKey);
    if (ffReadFileBuffer(pciDir->chars, buffer))
        gpu->frequency = (uint32_t) ffStrbufToUInt(buffer, 0);
    ffStrbufSubstrBefore(pciDir, pciDirLen);

    if (options->temp)
    {
        ffStrbufAppendS(pciDir, "/hwmon/");
        FF_AUTO_CLOSE_DIR DIR* dirp = opendir(pciDir->chars);
        if (dirp)
        {
            struct dirent* entry;
            while ((entry = readdir(dirp)) != NULL)
            {
                if (entry->d_name[0] == '.') continue;

                ffStrbufSubstrBefore(pciDir, pciDirLen + strlen("/hwmon/"));
                ffStrbufAppendS(pciDir, entry->d_name);
                // https://github.com/Syllo/nvtop/blob/73291884d926445e499d6b9b71cb7a9bdbc7c393/src/extract_gpuinfo_intel.c#L279-L281
                ffStrbufAppendS(pciDir, isXE ? "/temp2_input" : "/temp1_input");

                if (ffReadFileBuffer(pciDir->chars, buffer))
                {
                    uint64_t value = ffStrbufToUInt(buffer, 0);
                    if (value > 0)
                    {
                        gpu->temperature = (double) value / 1000;
                        break;
                    }
                }
            }
        }
        ffStrbufSubstrBefore(pciDir, pciDirLen);
    }
}

static const char* drmDetectIntelSpecific(FFGPUResult* gpu, const char* drmKey, FFstrbuf* buffer)
{
    #if FF_HAVE_DRM
    ffStrbufSetS(buffer, "/dev/dri/");
    ffStrbufAppendS(buffer, drmKey);
    FF_AUTO_CLOSE_FD int fd = open(buffer->chars, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return "Failed to open drm device";

    if (ffStrbufEqualS(&gpu->driver, "xe"))
        return ffDrmDetectXe(gpu, fd);
    else if (ffStrbufEqualS(&gpu->driver, "i915"))
        return ffDrmDetectI915(gpu, fd);
    return "Unknown Intel GPU driver";
    #else
    FF_UNUSED(gpu, drmKey, buffer);
    return "Fastfetch is not compiled with drm support";
    #endif
}

static const char* pciDetectNouveauSpecific(const FFGPUOptions* options, FFGPUResult* gpu, FFstrbuf* pciDir, FFstrbuf* buffer)
{
    if (options->temp)
    {
        const uint32_t pciDirLen = pciDir->length;
        ffStrbufAppendS(pciDir, "/hwmon/");
        FF_AUTO_CLOSE_DIR DIR* dirp = opendir(pciDir->chars);
        if (dirp)
        {
            struct dirent* entry;
            while ((entry = readdir(dirp)))
            {
                if (entry->d_name[0] == '.') continue;
                ffStrbufAppendS(pciDir, entry->d_name);
                ffStrbufAppendS(pciDir, "/temp1_input");
                if (ffReadFileBuffer(pciDir->chars, buffer))
                {
                    uint64_t value = ffStrbufToUInt(buffer, 0);
                    if (value > 0) gpu->temperature = (double) value / 1000.0;
                }
                break;
            }
        }
        ffStrbufSubstrBefore(pciDir, pciDirLen);
    }
    return NULL;
}

static const char* drmDetectNouveauSpecific(FFGPUResult* gpu, const char* drmKey, FFstrbuf* buffer)
{
    #if FF_HAVE_DRM
    ffStrbufSetS(buffer, "/dev/dri/");
    ffStrbufAppendS(buffer, drmKey);
    FF_AUTO_CLOSE_FD int fd = open(buffer->chars, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return "Failed to open drm device";

    return ffDrmDetectNouveau(gpu, fd);
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
        ssize_t pathLength = readlink(deviceDir->chars, pciPath, ARRAY_SIZE(pciPath) - 1);
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
        pPciPath = memrchr(deviceDir->chars, '/', deviceDir->length);
        assert(pPciPath);
        pPciPath++;
    }

    uint32_t pciDomain, pciBus, pciDevice, pciFunc;
    if (sscanf(pPciPath, "%" SCNx32 ":%" SCNx32 ":%" SCNx32 ".%" SCNx32, &pciDomain, &pciBus, &pciDevice, &pciFunc) != 4)
        return "Invalid PCI device path";

    FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
    ffStrbufInitStatic(&gpu->vendor, ffGPUGetVendorString((uint16_t) vendorId));
    ffStrbufInit(&gpu->name);
    ffStrbufInit(&gpu->driver);
    ffStrbufInit(&gpu->platformApi);
    ffStrbufInit(&gpu->memoryType);
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
                    ffStrCopy(drmKeyBuffer, entry->d_name, ARRAY_SIZE(drmKeyBuffer));
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
                    ffGPUQueryAmdGpuName((uint16_t) deviceId, (uint8_t) revision, gpu);
            }
            ffStrbufSubstrBefore(deviceDir, drmDirPathLength);
        }
    }
    else if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_INTEL)
    {
        pciDetectIntelSpecific(options, gpu, deviceDir, buffer, drmKey);
        ffStrbufSubstrBefore(deviceDir, drmDirPathLength);
        if (options->driverSpecific && drmKey)
            drmDetectIntelSpecific(gpu, drmKey, buffer);
    }
    else if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_NVIDIA && ffStrbufEqualS(&gpu->driver, "nouveau"))
    {
        pciDetectNouveauSpecific(options, gpu, deviceDir, buffer);
        if (options->driverSpecific && drmKey)
            drmDetectNouveauSpecific(gpu, drmKey, buffer);
    }
    else
    {
        ffGPUDetectDriverSpecific(options, gpu, (FFGpuDriverPciBusId) {
            .domain = pciDomain,
            .bus = pciBus,
            .device = pciDevice,
            .func = pciFunc,
        });
    }

    if (gpu->name.length == 0)
        ffGPUFillVendorAndName(subclassId, (uint16_t) vendorId, (uint16_t) deviceId, gpu);

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

    return NULL;
}

#if __aarch64__

FF_MAYBE_UNUSED static const char* drmDetectAsahiSpecific(FFGPUResult* gpu, const char* name, FF_MAYBE_UNUSED FFstrbuf* buffer, FF_MAYBE_UNUSED const char* drmKey)
{
    if (sscanf(name, "agx-t%lu", &gpu->deviceId) == 1)
        ffStrbufSetStatic(&gpu->name, ffCPUAppleCodeToName((uint32_t) gpu->deviceId));
    ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_APPLE);

    #if FF_HAVE_DRM_ASAHI
    ffStrbufSetS(buffer, "/dev/dri/");
    ffStrbufAppendS(buffer, drmKey);
    FF_AUTO_CLOSE_FD int fd = open(buffer->chars, O_RDONLY | O_CLOEXEC);
    if (fd >= 0)
        return ffDrmDetectAsahi(gpu, fd);
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
    ffStrbufInit(&gpu->memoryType);
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
