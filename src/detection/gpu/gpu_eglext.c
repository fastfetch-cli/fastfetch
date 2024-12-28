#include "detection/gpu/gpu.h"
#include "common/library.h"

#if defined(FF_HAVE_EGL) || __has_include(<EGL/egl.h>)
#include <EGL/egl.h>
#include <EGL/eglext.h>

const char* ffGPUDetectByEglext(FFlist* gpus)
{
    FF_LIBRARY_LOAD(egl, "dlopen libEGL" FF_LIBRARY_EXTENSION " failed", "libEGL" FF_LIBRARY_EXTENSION, 1);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(egl, eglGetProcAddress);

    PFNEGLQUERYDEVICESEXTPROC ffeglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC) ffeglGetProcAddress("eglQueryDevicesEXT");
    if (!ffeglQueryDevicesEXT)
        return "eglGetProcAddress(\"eglQueryDevicesEXT\") returned NULL";
    PFNEGLQUERYDEVICESTRINGEXTPROC ffeglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC) ffeglGetProcAddress("eglQueryDeviceStringEXT");
    if (!ffeglQueryDeviceStringEXT)
        return "eglGetProcAddress(\"eglQueryDeviceStringEXT\") returned NULL";

    EGLDeviceEXT devs[32];
    EGLint numDevs;
    if (ffeglQueryDevicesEXT(ARRAY_SIZE(devs), devs, &numDevs))
    {
        for (EGLint i = 0; i < numDevs; i++)
        {
            const char* renderer = ffeglQueryDeviceStringEXT(devs[i], EGL_RENDERER_EXT);
            if (!renderer) continue;

            FFGPUResult* gpu = (FFGPUResult*) ffListAdd(gpus);
            ffStrbufInitS(&gpu->name, renderer);
            ffStrbufInitS(&gpu->vendor, ffeglQueryDeviceStringEXT(devs[i], EGL_VENDOR));
            ffStrbufInitS(&gpu->driver, ffeglQueryDeviceStringEXT(devs[i], EGL_DRIVER_NAME_EXT));
            gpu->index = FF_GPU_INDEX_UNSET;
            gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
            gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
            gpu->temperature = FF_GPU_TEMP_UNSET;
            gpu->frequency = FF_GPU_FREQUENCY_UNSET;
            gpu->type = FF_GPU_TYPE_UNKNOWN;
            gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
            gpu->deviceId = 0;
            ffStrbufInitStatic(&gpu->platformApi, "EGLext");
        }
    }

    return NULL;
}

#else

const char* ffGPUDetectByEglext(FF_MAYBE_UNUSED FFlist* gpus)
{
    return "fastfetch was compiled without egl support";
}

#endif
