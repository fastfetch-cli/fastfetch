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
const char* FF_GPU_VENDOR_NAME_PARALLELS = "Parallels";
const char* FF_GPU_VENDOR_NAME_MICROSOFT = "Microsoft";
const char* FF_GPU_VENDOR_NAME_REDHAT = "RedHat";
const char* FF_GPU_VENDOR_NAME_ORACLE = "Oracle";
const char* FF_GPU_VENDOR_NAME_BROADCOM = "Broadcom";
const char* FF_GPU_VENDOR_NAME_LOONGSON = "Loongson";
const char* FF_GPU_VENDOR_NAME_JINGJIA_MICRO = "Jingjia Micro";
const char* FF_GPU_VENDOR_NAME_HUAWEI = "Huawei";
const char* FF_GPU_VENDOR_NAME_ZHAOXIN = "Zhaoxin";
const char* FF_GPU_VENDOR_NAME_QEMU = "QEMU";

const char* ffGPUGetVendorString(unsigned vendorId) {
    // https://devicehunt.com/all-pci-vendors
    switch (vendorId) {
        case 0x106b:
            return FF_GPU_VENDOR_NAME_APPLE;
        case 0x1002:
        case 0x1022:
        case 0x1dd8:
            return FF_GPU_VENDOR_NAME_AMD;
        case 0x8086:
        case 0x8087:
        case 0x03e7:
            return FF_GPU_VENDOR_NAME_INTEL;
        case 0x0955:
        case 0x10de:
        case 0x12d2:
            return FF_GPU_VENDOR_NAME_NVIDIA;
        case 0x1ed5:
            return FF_GPU_VENDOR_NAME_MTHREADS;
        case 0x17cb:
        case 0x5143:
            return FF_GPU_VENDOR_NAME_QUALCOMM;
        case 0x14c3:
            return FF_GPU_VENDOR_NAME_MTK;
        case 0x15ad:
            return FF_GPU_VENDOR_NAME_VMWARE;
        case 0x1af4:
            return FF_GPU_VENDOR_NAME_REDHAT;
        case 0x1ab8:
        case 0x05404c42: // PD
            return FF_GPU_VENDOR_NAME_PARALLELS;
        case 0x1414:
            return FF_GPU_VENDOR_NAME_MICROSOFT;
        case 0x108e:
            return FF_GPU_VENDOR_NAME_ORACLE;
        case 0x182f:
        case 0x14e4:
            return FF_GPU_VENDOR_NAME_BROADCOM;
        case 0x0014:
            return FF_GPU_VENDOR_NAME_LOONGSON;
        case 0x0731:
            return FF_GPU_VENDOR_NAME_JINGJIA_MICRO;
        case 0x19e5:
            return FF_GPU_VENDOR_NAME_HUAWEI;
        case 0x1d17:
            return FF_GPU_VENDOR_NAME_ZHAOXIN;
        case 0x1234: // https://admin.pci-ids.ucw.cz/read/PC/1234
            return FF_GPU_VENDOR_NAME_QEMU;
        default:
            return NULL;
    }
}

static bool normalizeVendorName(FFGPUResult* gpu) {
    if (ffStrbufContainS(&gpu->name, "Apple")) {
        ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_APPLE);
        gpu->type = FF_GPU_TYPE_INTEGRATED;
    } else if (ffStrbufContainS(&gpu->name, "Intel")) {
        ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
    } else if (ffStrbufContainS(&gpu->name, "AMD") || ffStrbufContainS(&gpu->name, "ATI")) {
        ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD);
    } else if (ffStrbufContainS(&gpu->name, "NVIDIA")) {
        ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_NVIDIA);
    } else if (ffStrbufContainS(&gpu->name, "MTT")) {
        ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_MTHREADS);
    } else {
        return false;
    }

    return true;
}

const char* detectByOpenGL(FFlist* gpus) {
    FF_DEBUG("Starting OpenGL GPU detection fallback");

    FFOpenGLResult result;
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.renderer);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.slv);
    ffStrbufInit(&result.library);

    FF_A_CLEANUP(ffDestroyOpenGLOptions) FFOpenGLOptions options;
    ffInitOpenGLOptions(&options);
    const char* error = ffDetectOpenGL(&options, &result);
    FF_DEBUG("OpenGL detection returns: %s", error ?: "success");

    if (!error) {
        FFGPUResult* gpu = FF_LIST_ADD(FFGPUResult, *gpus);
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
        gpu->dedicated = gpu->shared = (FFGPUMemory) { 0, 0 };
        gpu->deviceId = 0;
        gpu->pcieSpeed = FF_GPU_PCIE_SPEED_UNSET;

        FF_DEBUG("OpenGL reported renderer='%s', vendor='%s', version='%s'",
            gpu->name.chars,
            gpu->vendor.chars,
            result.version.chars);

        normalizeVendorName(gpu);

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

#if defined(FF_HAVE_EGL) || __has_include(<EGL/egl.h>)
    #include <EGL/egl.h>
    #include <EGL/eglext.h>

    #if EGL_EXT_device_base && EGL_EXT_device_persistent_id && EGL_EXT_device_query_name

        #define FF_HAVE_EGL_EXT 1
        #include <inttypes.h>

        #include "common/library.h"
        #include "common/strutil.h"
        #include "common/io.h"

const char* detectByEglext(FFlist* result) {
    FF_LIBRARY_LOAD_MESSAGE(egl, "libEGL" FF_LIBRARY_EXTENSION, 1);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(egl, eglGetProcAddress);

    PFNEGLQUERYDEVICESEXTPROC ffeglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC) ffeglGetProcAddress("eglQueryDevicesEXT");
    if (!ffeglQueryDevicesEXT) {
        return "eglQueryDevicesEXT is unavailable";
    }
    PFNEGLQUERYDEVICESTRINGEXTPROC ffeglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC) ffeglGetProcAddress("eglQueryDeviceStringEXT");
    if (!ffeglQueryDeviceStringEXT) {
        return "eglQueryDeviceStringEXT is unavailable";
    }
    PFNEGLQUERYDEVICEBINARYEXTPROC ffeglQueryDeviceBinaryEXT = (PFNEGLQUERYDEVICEBINARYEXTPROC) ffeglGetProcAddress("eglQueryDeviceBinaryEXT");
    if (!ffeglQueryDeviceBinaryEXT) {
        return "eglQueryDeviceBinaryEXT is unavailable";
    }

    FF_DEBUG("Loaded EGL library and required symbols");

    FF_SUPPRESS_IO();

    EGLDeviceEXT devices[16];
    EGLint numDevices;
    if (ffeglQueryDevicesEXT(ARRAY_SIZE(devices), devices, &numDevices) == EGL_TRUE) {
        FF_DEBUG("eglQueryDevicesEXT() returned %d device(s)", numDevices);
        for (EGLint i = 0; i < numDevices; i++) {
            const EGLDeviceEXT device = devices[i];
            if (device == (EGLDeviceEXT) EGL_NO_DEVICE_EXT || device == (EGLDeviceEXT) EGL_BAD_DEVICE_EXT) {
                FF_DEBUG("eglQueryDevicesEXT() returned invalid device at index %d", i);
                continue;
            }
            const char* exts = (const char*) ffeglQueryDeviceStringEXT(device, EGL_EXTENSIONS);
            FF_DEBUG("eglQueryDeviceStringEXT() returned extensions for device %d: %s", i, exts);
            if (exts && ffStrContains(exts, "EGL_MESA_device_software")) {
                FF_DEBUG("eglQueryDeviceStringEXT() returned software device at index %d, skipping", i);
                continue;
            }

            const char* name = ffeglQueryDeviceStringEXT(device, EGL_RENDERER_EXT);
            if (!name) {
                FF_DEBUG("eglQueryDeviceStringEXT() returned NULL name for device %d, skipping", i);
                continue;
            }

            FFGPUResult* gpu = FF_LIST_ADD(FFGPUResult, *result);
            ffStrbufInitS(&gpu->name, name);
            ffStrbufInit(&gpu->vendor);
            ffStrbufInitS(&gpu->driver, ffeglQueryDeviceStringEXT(device, EGL_DRIVER_NAME_EXT));
            ffStrbufInitF(&gpu->platformApi, "egl-ext");
            ffStrbufInit(&gpu->memoryType);
            gpu->index = FF_GPU_INDEX_UNSET;
            gpu->temperature = FF_GPU_TEMP_UNSET;
            gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
            gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
            gpu->type = FF_GPU_TYPE_UNKNOWN;
            gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
            gpu->deviceId = 0;
            gpu->frequency = FF_GPU_FREQUENCY_UNSET;
            gpu->pcieSpeed = FF_GPU_PCIE_SPEED_UNSET;

            ffeglQueryDeviceBinaryEXT(device, EGL_DEVICE_UUID_EXT, sizeof(gpu->deviceId), &gpu->deviceId, NULL);

            if (!normalizeVendorName(gpu)) {
                ffStrbufSetS(&gpu->vendor, ffeglQueryDeviceStringEXT(device, EGL_VENDOR));
            }

            FF_DEBUG("Detected GPU %d: vendor='%s', name='%s', driver='%s', id='%16" PRIX64 "'", i, gpu->vendor.chars, gpu->name.chars, gpu->driver.chars, gpu->deviceId);
        }
    } else {
        FF_DEBUG("eglQueryDevicesEXT() returned EGL_FALSE");
    }

    return NULL;
}
    #endif

#endif


const char* ffDetectGPU(const FFGPUOptions* options, FFlist* result) {
    FF_DEBUG("Starting GPU detection with method=%d", (int) options->detectionMethod);

    if (options->detectionMethod <= FF_GPU_DETECTION_METHOD_PCI) {
        FF_DEBUG("Trying PCI/native GPU detection");
        const char* error = ffDetectGPUImpl(options, result);
        if (!error && result->length > 0) {
            FF_DEBUG("PCI/native GPU detection succeeded with %u GPU(s)", result->length);
            return NULL;
        }

        FF_DEBUG("PCI/native GPU detection did not produce results (error=%s, gpuCount=%u)",
            error ?: "none",
            result->length);
    }
    if (options->detectionMethod <= FF_GPU_DETECTION_METHOD_VULKAN) {
        FF_DEBUG("Trying Vulkan GPU detection fallback");
        FFVulkanResult* vulkan = ffDetectVulkan();
        if (!vulkan->error && vulkan->gpus.length > 0) {
            FF_DEBUG("Vulkan detection succeeded with %u GPU(s)", vulkan->gpus.length);
            ffListDestroy(result);
            ffListInitMove(result, &vulkan->gpus);

#ifdef __ANDROID__
            double ffGPUDetectTempFromTZ(void);
            if (options->temp && result->length == 1) {
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
    if (options->detectionMethod <= FF_GPU_DETECTION_METHOD_OPENCL) {
        FF_DEBUG("Trying OpenCL GPU detection fallback");
        FFOpenCLResult* opencl = ffDetectOpenCL();
        if (!opencl->error && opencl->gpus.length > 0) {
            FF_DEBUG("OpenCL detection succeeded with %u GPU(s)", opencl->gpus.length);
            ffListDestroy(result);
            ffListInitMove(result, &opencl->gpus);
            return NULL;
        }

        FF_DEBUG("OpenCL detection did not produce results (error=%s, gpuCount=%u)",
            opencl->error ?: "none",
            opencl->gpus.length);
    }
    if (options->detectionMethod <= FF_GPU_DETECTION_METHOD_EGL_EXT) {
        #if FF_HAVE_EGL_EXT
        FF_DEBUG("Trying EGL_EXT GPU detection fallback");
        const char* error = detectByEglext(result);
        if (!error && result->length > 0) {
            FF_DEBUG("EGL_EXT GPU detection succeeded with %u GPU(s)", result->length);
            return NULL;
        }

        FF_DEBUG("EGL_EXT GPU detection did not produce results (error=%s, gpuCount=%u)",
            error ?: "none",
            result->length);
        #else
        FF_DEBUG("EGL_EXT GPU detection is unavailable");
        #endif
    }
    if (options->detectionMethod <= FF_GPU_DETECTION_METHOD_OPENGL) {
        FF_DEBUG("Trying OpenGL GPU detection fallback");
        const char* error = detectByOpenGL(result);
        if (error == NULL) {
            FF_DEBUG("OpenGL fallback succeeded with %u GPU(s)", result->length);
            return NULL;
        }

        FF_DEBUG("OpenGL fallback failed: %s", error);
    }

    FF_DEBUG("GPU detection failed in all enabled backends");
    return "GPU detection failed";
}

bool ffGPUFillVendorByDeviceName(FFGPUResult* gpu) {
    if (gpu->type != FF_GPU_TYPE_UNKNOWN) {
        return true;
    }

    FF_DEBUG("Using fallback GPU type detection");
    if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_NVIDIA) {
        if (ffStrbufStartsWithIgnCaseS(&gpu->name, "GeForce") ||
            ffStrbufStartsWithIgnCaseS(&gpu->name, "Quadro") ||
            ffStrbufStartsWithIgnCaseS(&gpu->name, "Tesla")) {
            gpu->type = FF_GPU_TYPE_DISCRETE;
        }
    } else if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_MTHREADS) {
        if (ffStrbufStartsWithIgnCaseS(&gpu->name, "MTT ")) { gpu->type = FF_GPU_TYPE_DISCRETE; }
    } else if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_INTEL) {
        // 0000:00:02.0 is reserved for Intel integrated graphics
        gpu->type = gpu->deviceId == ffGPUPciAddr2Id(0, 0, 2, 0) ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;
    } else if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_VMWARE || gpu->vendor.chars == FF_GPU_VENDOR_NAME_PARALLELS) {
        // Virtualized GPUs
        gpu->type = FF_GPU_TYPE_INTEGRATED;
    }

    if (gpu->type != FF_GPU_TYPE_UNKNOWN) {
        FF_DEBUG("Determined GPU type based on vendor (%s) and name: %u", gpu->vendor.chars, gpu->type);
        return true;
    }

    return false;
}
