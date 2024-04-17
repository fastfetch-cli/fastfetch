#include "detection/gpu/gpu.h"
#include "detection/vulkan/vulkan.h"
#include "detection/temps/temps_linux.h"
#include "common/io/io.h"
#include "common/properties.h"
#include "util/stringUtils.h"

#ifdef FF_USE_PROPRIETARY_GPU_DRIVER_API
    #include "detection/gpu/gpu_driver_specific.h"
#endif

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

#include <inttypes.h>

FF_MAYBE_UNUSED static void pciDetectTemp(FFGPUResult* gpu, uint32_t deviceClass)
{
    const FFlist* tempsResult = ffDetectTemps();

    FF_LIST_FOR_EACH(FFTempValue, tempValue, *tempsResult)
    {
        // https://www.kernel.org/doc/html/v5.10/gpu/amdgpu.html#hwmon-interfaces
        // FIXME: this code doesn't take multiGPUs into count
        // The kernel exposes the device class multiplied by 256 for some reason
        if(tempValue->deviceClass == deviceClass * 256)
        {
            gpu->temperature = tempValue->value;
            return;
        }
    }
}

static void pciDetectDriver(FFGPUResult* gpu, FFstrbuf* pciDir, FFstrbuf* buffer)
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
        }
    }
}

static void pciDetectVmem(FFGPUResult* gpu, FFstrbuf* pciDir, FFstrbuf* buffer)
{
    // https://www.kernel.org/doc/html/v5.10/gpu/amdgpu.html#mem-info-vis-vram-total
    ffStrbufAppendS(pciDir, "/mem_info_vis_vram_total");
    uint64_t size = 0;
    if (ffReadFileBuffer(pciDir->chars, buffer) && (size = ffStrbufToUInt(buffer, 0)))
    {
        gpu->type = size > 1024UL * 1024 * 1024 ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;
        if (gpu->type == FF_GPU_TYPE_DISCRETE)
            gpu->dedicated.total = size;
        else
            gpu->shared.total = size;

        ffStrbufSubstrBefore(pciDir, pciDir->length - (uint32_t) strlen("/mem_info_vis_vram_total"));
        ffStrbufAppendS(pciDir, "/mem_info_vram_used");
        if (ffReadFileBuffer(pciDir->chars, buffer) && (size = ffStrbufToUInt(buffer, 0)))
        {
            if (gpu->type == FF_GPU_TYPE_DISCRETE)
                gpu->dedicated.used = size;
            else
                gpu->shared.used = size;
        }
    }
}

static bool loadPciIds(FFstrbuf* pciids)
{
    #ifdef FF_CUSTOM_PCI_IDS_PATH
    ffReadFileBuffer(FF_STR(FF_CUSTOM_PCI_IDS_PATH), pciids);
    if (pciids->length > 0) return true;
    #endif

    ffReadFileBuffer(FASTFETCH_TARGET_DIR_USR "/share/hwdata/pci.ids", pciids);
    if (pciids->length > 0) return true;

    ffReadFileBuffer(FASTFETCH_TARGET_DIR_USR "/share/misc/pci.ids", pciids); // debian?
    if (pciids->length > 0) return true;

    ffReadFileBuffer(FASTFETCH_TARGET_DIR_USR "/local/share/hwdata/pci.ids", pciids);
    if (pciids->length > 0) return true;

    return false;
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
    FF_STRBUF_AUTO_DESTROY pciids = ffStrbufCreate();

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

        uint32_t vendorId, deviceId, subVendorId, subDeviceId;
        uint8_t classId, subclassId;
        if (sscanf(buffer.chars, "pci:v%8" SCNx32 "d%8" SCNx32 "sv%8" SCNx32 "sd%8" SCNx32 "bc%2" SCNx8 "sc%2" SCNx8, &vendorId, &deviceId, &subVendorId, &subDeviceId, &classId, &subclassId) != 6)
            continue;

        if (classId != 0x03 /*PCI_BASE_CLASS_DISPLAY*/)
            continue;

        uint32_t pciDomain, pciBus, pciDevice, pciFunc;
        if (sscanf(entry->d_name, "%" SCNx32 ":%" SCNx32 ":%" SCNx32 ".%" SCNx32, &pciDomain, &pciBus, &pciDevice, &pciFunc) != 4)
            continue;

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

        if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
        {
            ffStrbufAppendS(&pciDir, "/revision");
            if (ffReadFileBuffer(pciDir.chars, &buffer))
            {
                char* pend;
                uint64_t revision = strtoul(buffer.chars, &pend, 16);
                if (pend != buffer.chars)
                {
                    char query[32];
                    snprintf(query, sizeof(query), "%X,\t%X,", (unsigned) deviceId, (unsigned) revision);
                    ffParsePropFileData("libdrm/amdgpu.ids", query, &gpu->name);
                }
            }
            ffStrbufSubstrBefore(&pciDir, pciDevDirLength);
        }

        if (gpu->name.length == 0)
        {
            if (!pciids.length)
                loadPciIds(&pciids);
            ffGPUParsePciIds(&pciids, subclassId, (uint16_t) vendorId, (uint16_t) deviceId, gpu);
        }

        pciDetectVmem(gpu, &pciDir, &buffer);
        ffStrbufSubstrBefore(&pciDir, pciDevDirLength);

        pciDetectDriver(gpu, &pciDir, &buffer);
        ffStrbufSubstrBefore(&pciDir, pciDevDirLength);

        #ifdef FF_USE_PROPRIETARY_GPU_DRIVER_API
        if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_NVIDIA && (options->temp || options->driverSpecific))
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

            if (gpu->dedicated.total != FF_GPU_VMEM_SIZE_UNSET)
                gpu->type = gpu->dedicated.total > (uint64_t)1024 * 1024 * 1024 ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;
        }
        #endif // FF_USE_PROPRIETARY_GPU_DRIVER_API

        #ifdef __linux__
        if(options->temp && gpu->temperature != gpu->temperature)
            pciDetectTemp(gpu, ((uint32_t) classId << 8) + subclassId);
        #endif
    }

    return NULL;
}

const char* ffDetectGPUImpl(const FFGPUOptions* options, FFlist* gpus)
{
    #ifdef FF_HAVE_DIRECTX_HEADERS
        const char* ffGPUDetectByDirectX(const FFGPUOptions* options, FFlist* gpus);
        if (!ffGPUDetectByDirectX(options, gpus))
            return NULL;
    #endif

    return pciDetectGPUs(options, gpus);
}
