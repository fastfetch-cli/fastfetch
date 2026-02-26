#include "gpu.h"
#include "common/debug.h"
#include "detection/vulkan/vulkan.h"
#include "detection/opencl/opencl.h"
#include "detection/opengl/opengl.h"
#include "modules/opengl/opengl.h"

const char* FF_GPU_VENDOR_NAME_APPLE = "Apple";
const char* FF_GPU_VENDOR_NAME_AMD = "AMD";
const char* FF_GPU_VENDOR_NAME_INTEL = "Intel";
const char* FF_GPU_VENDOR_NAME_NVIDIA = "NVIDIA";
const char* FF_GPU_VENDOR_NAME_MTHREADS = "Moore Threads";
const char* FF_GPU_VENDOR_NAME_QUALCOMM = "Qualcomm";
const char* FF_GPU_VENDOR_NAME_MTK = "MTK";
const char* FF_GPU_VENDOR_NAME_VMWARE = "VMware";
const char* FF_GPU_VENDOR_NAME_PARALLEL = "Parallel";
const char* FF_GPU_VENDOR_NAME_MICROSOFT = "Microsoft";
const char* FF_GPU_VENDOR_NAME_REDHAT = "RedHat";
const char* FF_GPU_VENDOR_NAME_ORACLE = "Oracle";
const char* FF_GPU_VENDOR_NAME_BROADCOM = "Broadcom";
const char* FF_GPU_VENDOR_NAME_LOONGSON = "Loongson";
const char* FF_GPU_VENDOR_NAME_JINGJIA_MICRO = "Jingjia Micro";
const char* FF_GPU_VENDOR_NAME_HUAWEI = "Huawei";
const char* FF_GPU_VENDOR_NAME_ZHAOXIN = "Zhaoxin";

const char* ffGPUGetVendorString(unsigned vendorId)
{
    // https://devicehunt.com/all-pci-vendors
    switch (vendorId)
    {
        case 0x106b: return FF_GPU_VENDOR_NAME_APPLE;
        case 0x1002: case 0x1022: case 0x1dd8: return FF_GPU_VENDOR_NAME_AMD;
        case 0x8086: case 0x8087: case 0x03e7: return FF_GPU_VENDOR_NAME_INTEL;
        case 0x0955: case 0x10de: case 0x12d2: return FF_GPU_VENDOR_NAME_NVIDIA;
        case 0x1ed5: return FF_GPU_VENDOR_NAME_MTHREADS;
        case 0x17cb: case 0x5143: return FF_GPU_VENDOR_NAME_QUALCOMM;
        case 0x14c3: return FF_GPU_VENDOR_NAME_MTK;
        case 0x15ad: return FF_GPU_VENDOR_NAME_VMWARE;
        case 0x1af4: return FF_GPU_VENDOR_NAME_REDHAT;
        case 0x1ab8: return FF_GPU_VENDOR_NAME_PARALLEL;
        case 0x1414: return FF_GPU_VENDOR_NAME_MICROSOFT;
        case 0x108e: return FF_GPU_VENDOR_NAME_ORACLE;
        case 0x182f: case 0x14e4: return FF_GPU_VENDOR_NAME_BROADCOM;
        case 0x0014: return FF_GPU_VENDOR_NAME_LOONGSON;
        case 0x0731: return FF_GPU_VENDOR_NAME_JINGJIA_MICRO;
        case 0x19e5: return FF_GPU_VENDOR_NAME_HUAWEI;
        case 0x1d17: return FF_GPU_VENDOR_NAME_ZHAOXIN;
        default: return NULL;
    }
}

const char* detectByOpenGL(FFlist* gpus)
{
    FF_DEBUG("Starting OpenGL GPU detection fallback");

    FFOpenGLResult result;
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.renderer);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.slv);
    ffStrbufInit(&result.library);

    __attribute__((__cleanup__(ffDestroyOpenGLOptions))) FFOpenGLOptions options;
    ffInitOpenGLOptions(&options);
    const char* error = ffDetectOpenGL(&options, &result);
    FF_DEBUG("OpenGL detection returns: %s", error ?: "success");

    if (!error)
    {
        FFGPUResult* gpu = (FFGPUResult*) ffListAdd(gpus);
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        ffStrbufInitMove(&gpu->vendor, &result.vendor);
        ffStrbufInitMove(&gpu->name, &result.renderer);
        ffStrbufInit(&gpu->driver);
        ffStrbufInitF(&gpu->platformApi, "OpenGL %s", result.version.chars);
        ffStrbufInit(&gpu->memoryType);
        gpu->index = FF_GPU_INDEX_UNSET;
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;
        gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
        gpu->dedicated = gpu->shared = (FFGPUMemory){0, 0};
        gpu->deviceId = 0;

        FF_DEBUG("OpenGL reported renderer='%s', vendor='%s', version='%s'",
            gpu->name.chars,
            gpu->vendor.chars,
            result.version.chars);

        if (ffStrbufContainS(&gpu->name, "Apple"))
        {
            ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_APPLE);
            gpu->type = FF_GPU_TYPE_INTEGRATED;
        }
        else if (ffStrbufContainS(&gpu->name, "Intel"))
            ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
        else if (ffStrbufContainS(&gpu->name, "AMD") || ffStrbufContainS(&gpu->name, "ATI"))
            ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD);
        else if (ffStrbufContainS(&gpu->name, "NVIDIA"))
            ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_NVIDIA);
        else if (ffStrbufContainS(&gpu->name, "MTT"))
            ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_MTHREADS);

        FF_DEBUG("OpenGL fallback produced GPU: name='%s', vendor='%s', type=%u",
            gpu->name.chars,
            gpu->vendor.chars,
            gpu->type);

    }

    ffStrbufDestroy(&result.version);
    ffStrbufDestroy(&result.renderer);
    ffStrbufDestroy(&result.vendor);
    ffStrbufDestroy(&result.slv);
    ffStrbufDestroy(&result.library);
    return error;
}

const char* ffDetectGPU(const FFGPUOptions* options, FFlist* result)
{
    FF_DEBUG("Starting GPU detection with method=%d", (int) options->detectionMethod);

    if (options->detectionMethod <= FF_GPU_DETECTION_METHOD_PCI)
    {
        FF_DEBUG("Trying PCI/native GPU detection");
        const char* error = ffDetectGPUImpl(options, result);
        if (!error && result->length > 0)
        {
            FF_DEBUG("PCI/native GPU detection succeeded with %u GPU(s)", result->length);
            return NULL;
        }

        FF_DEBUG("PCI/native GPU detection did not produce results (error=%s, gpuCount=%u)",
            error ?: "none",
            result->length);
    }
    if (options->detectionMethod <= FF_GPU_DETECTION_METHOD_VULKAN)
    {
        FF_DEBUG("Trying Vulkan GPU detection fallback");
        FFVulkanResult* vulkan = ffDetectVulkan();
        if (!vulkan->error && vulkan->gpus.length > 0)
        {
            FF_DEBUG("Vulkan detection succeeded with %u GPU(s)", vulkan->gpus.length);
            ffListDestroy(result);
            ffListInitMove(result, &vulkan->gpus);

            #ifdef __ANDROID__
            double ffGPUDetectTempFromTZ(void);
            if (options->temp && result->length == 1)
            {
                FF_DEBUG("Applying Android thermal-zone temperature to single Vulkan GPU");
                FF_LIST_GET(FFGPUResult, *result, 0)->temperature = ffGPUDetectTempFromTZ();
            }
            #endif

            return NULL;
        }

        FF_DEBUG("Vulkan detection did not produce results (error=%s, gpuCount=%u)",
            vulkan->error ?: "none",
            vulkan->gpus.length);
    }
    if (options->detectionMethod <= FF_GPU_DETECTION_METHOD_OPENCL)
    {
        FF_DEBUG("Trying OpenCL GPU detection fallback");
        FFOpenCLResult* opencl = ffDetectOpenCL();
        if (!opencl->error && opencl->gpus.length > 0)
        {
            FF_DEBUG("OpenCL detection succeeded with %u GPU(s)", opencl->gpus.length);
            ffListDestroy(result);
            ffListInitMove(result, &opencl->gpus);
            return NULL;
        }

        FF_DEBUG("OpenCL detection did not produce results (error=%s, gpuCount=%u)",
            opencl->error ?: "none",
            opencl->gpus.length);
    }
    if (options->detectionMethod <= FF_GPU_DETECTION_METHOD_OPENGL)
    {
        FF_DEBUG("Trying OpenGL GPU detection fallback");
        const char* error = detectByOpenGL(result);
        if (error == NULL)
        {
            FF_DEBUG("OpenGL fallback succeeded with %u GPU(s)", result->length);
            return NULL;
        }

        FF_DEBUG("OpenGL fallback failed: %s", error);
    }

    FF_DEBUG("GPU detection failed in all enabled backends");
    return "GPU detection failed";
}
