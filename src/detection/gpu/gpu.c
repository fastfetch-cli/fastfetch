#include "gpu.h"
#include "detection/vulkan/vulkan.h"
#include "detection/opengl/opengl.h"

const char* FF_GPU_VENDOR_NAME_APPLE = "Apple";
const char* FF_GPU_VENDOR_NAME_AMD = "AMD";
const char* FF_GPU_VENDOR_NAME_INTEL = "Intel";
const char* FF_GPU_VENDOR_NAME_NVIDIA = "NVIDIA";
const char* FF_GPU_VENDOR_NAME_QUALCOMM = "Qualcomm";
const char* FF_GPU_VENDOR_NAME_MTK = "MTK";
const char* FF_GPU_VENDOR_NAME_VMWARE = "VMware";
const char* FF_GPU_VENDOR_NAME_PARALLEL = "Parallel";
const char* FF_GPU_VENDOR_NAME_MICROSOFT = "Microsoft";
const char* FF_GPU_VENDOR_NAME_REDHAT = "RedHat";
const char* FF_GPU_VENDOR_NAME_ORACLE = "Oracle";

const char* ffGetGPUVendorString(unsigned vendorId)
{
    // https://devicehunt.com/all-pci-vendors
    switch (vendorId)
    {
        case 0x106b: return FF_GPU_VENDOR_NAME_APPLE;
        case 0x1002: case 0x1022: return FF_GPU_VENDOR_NAME_AMD;
        case 0x8086: case 0x8087: case 0x03e7: return FF_GPU_VENDOR_NAME_INTEL;
        case 0x0955: case 0x10de: case 0x12d2: return FF_GPU_VENDOR_NAME_NVIDIA;
        case 0x5143: return FF_GPU_VENDOR_NAME_QUALCOMM;
        case 0x14c3: return FF_GPU_VENDOR_NAME_MTK;
        case 0x15ad: return FF_GPU_VENDOR_NAME_VMWARE;
        case 0x1af4: return FF_GPU_VENDOR_NAME_REDHAT;
        case 0x1ab8: return FF_GPU_VENDOR_NAME_PARALLEL;
        case 0x1414: return FF_GPU_VENDOR_NAME_MICROSOFT;
        case 0x108e: return FF_GPU_VENDOR_NAME_ORACLE;
        default: return NULL;
    }
}

const char* detectByOpenGL(FFlist* gpus)
{
    FFOpenGLResult result;
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.renderer);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.slv);
    const char* error = ffDetectOpenGL(&instance.config.modules.openGL, &result);
    if (!error)
    {
        FFGPUResult* gpu = (FFGPUResult*) ffListAdd(gpus);
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        ffStrbufInit(&gpu->vendor);
        ffStrbufInitMove(&gpu->name, &result.renderer);
        ffStrbufInitMove(&gpu->driver, &result.vendor);
        ffStrbufInitF(&gpu->platformApi, "OpenGL %s", result.version.chars);
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;
        gpu->dedicated = gpu->shared = (FFGPUMemory){0, 0};
        gpu->deviceId = 0;

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

    }

    ffStrbufDestroy(&result.version);
    ffStrbufDestroy(&result.renderer);
    ffStrbufDestroy(&result.vendor);
    ffStrbufDestroy(&result.slv);
    return error;
}

const char* ffDetectGPU(const FFGPUOptions* options, FFlist* result)
{
    if (options->detectionMethod <= FF_GPU_DETECTION_METHOD_PCI)
    {
        const char* error = ffDetectGPUImpl(options, result);
        if (!error && result->length > 0) return NULL;
    }
    if (options->detectionMethod <= FF_GPU_DETECTION_METHOD_VULKAN)
    {
        FFVulkanResult* vulkan = ffDetectVulkan();
        if (!vulkan->error && vulkan->gpus.length > 0)
        {
            ffListDestroy(result);
            ffListInitMove(result, &vulkan->gpus);
            return NULL;
        }
    }
    if (options->detectionMethod <= FF_GPU_DETECTION_METHOD_OPENGL)
    {
        if (detectByOpenGL(result) == NULL)
            return NULL;
    }

    return "GPU detection failed";
}
