#include "detection/opencl/opencl.h"

#if !defined(FF_HAVE_OPENCL) && defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_15)
    #define FF_HAVE_OPENCL 1
#endif

#ifdef FF_HAVE_OPENCL

#include "common/library.h"
#include "common/parsing.h"
#include "detection/gpu/gpu.h"
#include "util/stringUtils.h"
#include <string.h>

#define CL_TARGET_OPENCL_VERSION 100
#ifndef __APPLE__
    #include <CL/cl.h>
#else
    #include <OpenCL/cl.h>
#endif

typedef struct OpenCLData
{
    FF_LIBRARY_SYMBOL(clGetPlatformInfo)
    FF_LIBRARY_SYMBOL(clGetDeviceInfo)
    FF_LIBRARY_SYMBOL(clGetDeviceIDs)
} OpenCLData;

static const char* openCLHandleData(OpenCLData* data, FFOpenCLResult* result)
{
    char buffer[1024] = "";
    if (data->ffclGetPlatformInfo(NULL, CL_PLATFORM_VERSION, sizeof(buffer), buffer, NULL) != CL_SUCCESS)
        return "clGetPlatformInfo(NULL, CL_PLATFORM_VERSION) failed";

    {
        const char* versionPretty = buffer;
        if(ffStrStartsWithIgnCase(buffer, "OpenCL "))
            versionPretty = buffer + strlen("OpenCL ");
        ffStrbufSetS(&result->version, versionPretty);
        ffStrbufTrim(&result->version, ' ');
    }

    if (clGetPlatformInfo(NULL, CL_PLATFORM_NAME, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
        ffStrbufSetS(&result->name, buffer);

    if (clGetPlatformInfo(NULL, CL_PLATFORM_VENDOR, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
        ffStrbufSetS(&result->vendor, buffer);

    cl_device_id deviceIDs[32];
    cl_uint numDevices = (cl_uint) (sizeof(deviceIDs) / sizeof(deviceIDs[0]));
    data->ffclGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, numDevices, deviceIDs, &numDevices);

    for (cl_uint index = 0; index < numDevices; ++index)
    {
        cl_device_id deviceID = deviceIDs[index];
        if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_NAME, sizeof(buffer), buffer, NULL) != CL_SUCCESS)
            continue;

        FFGPUResult* gpu = ffListAdd(&result->gpus);
        ffStrbufInitS(&gpu->name, buffer);
        ffStrbufInit(&gpu->vendor);
        ffStrbufInit(&gpu->driver);
        ffStrbufInit(&gpu->platformApi);
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = index + 1;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_VERSION, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
        {
            ffStrbufSetS(&gpu->platformApi, buffer);
            ffStrbufTrimRight(&gpu->platformApi, ' ');
        }
        else
            ffStrbufSetS(&gpu->platformApi, "OpenCL");

        if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_VENDOR, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
            ffStrbufSetS(&gpu->vendor, buffer);

        if (data->ffclGetDeviceInfo(deviceID, CL_DRIVER_VERSION, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
        {
            const char* versionPretty = strchr(buffer, ' ');
            if (versionPretty)
                ffStrbufSetS(&gpu->driver, versionPretty + 1);
            else
                ffStrbufSetS(&gpu->driver, buffer);
        }

        if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
            gpu->coreCount = (int32_t)*(cl_uint*) buffer;

        if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
            gpu->frequency = (*(cl_uint*) buffer) / 1000.;

        if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
            gpu->type = *(cl_bool*) buffer == CL_TRUE ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;

        if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
        {
            if (*(cl_device_local_mem_type*) buffer == CL_LOCAL)
            {
                if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
                    gpu->dedicated.total = *(cl_ulong*) buffer;
            }
        }

        if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
            gpu->shared.total = *(cl_ulong*) buffer;
    }

    return NULL;
}

static const char* detectOpenCL(FFOpenCLResult* result)
{
    OpenCLData data;

    #ifndef __APPLE__

    FF_LIBRARY_LOAD(opencl, &instance.config.library.libOpenCL, "dlopen libOpenCL" FF_LIBRARY_EXTENSION" failed",
    #ifdef _WIN32
        "OpenCL"FF_LIBRARY_EXTENSION, -1,
    #endif
        "libOpenCL"FF_LIBRARY_EXTENSION, 1
    );
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opencl, data, clGetPlatformInfo);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opencl, data, clGetDeviceIDs);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opencl, data, clGetDeviceInfo);

    return openCLHandleData(&data, result);

    #else

    data.ffclGetPlatformInfo = clGetPlatformInfo;
    data.ffclGetDeviceIDs = clGetDeviceIDs;
    data.ffclGetDeviceInfo = clGetDeviceInfo;

    return openCLHandleData(&data, result);

    #endif
}

#endif // defined(FF_HAVE_OPENCL)

FFOpenCLResult* ffDetectOpenCL(void)
{
    static FFOpenCLResult result;

    if (result.gpus.elementSize == 0)
    {
        ffStrbufInit(&result.version);
        ffStrbufInit(&result.name);
        ffStrbufInit(&result.vendor);
        ffListInit(&result.gpus, sizeof(FFGPUResult));

        #ifdef FF_HAVE_OPENCL
            result.error = detectOpenCL(&result);
        #else
            result.error = "fastfetch was compiled without OpenCL support";
        #endif
    }

    return &result;
}
