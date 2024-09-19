#include "detection/opencl/opencl.h"
#include "detection/gpu/gpu.h"

#if !defined(FF_HAVE_OPENCL) && defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_15)
    #define FF_HAVE_OPENCL 1
#endif

#ifdef FF_HAVE_OPENCL

#include "common/library.h"
#include "common/parsing.h"
#include "util/stringUtils.h"
#include <string.h>

#define CL_TARGET_OPENCL_VERSION 110
#ifndef __APPLE__
    #include <CL/cl.h>
#else
    #include <OpenCL/cl.h>
#endif

typedef struct OpenCLData
{
    FF_LIBRARY_SYMBOL(clGetPlatformIDs)
    FF_LIBRARY_SYMBOL(clGetPlatformInfo)
    FF_LIBRARY_SYMBOL(clGetDeviceInfo)
    FF_LIBRARY_SYMBOL(clGetDeviceIDs)
} OpenCLData;

static const char* openCLHandleData(OpenCLData* data, FFOpenCLResult* result)
{
    cl_platform_id platforms[32];
    cl_uint numPlatforms = 0;
    if (data->ffclGetPlatformIDs(sizeof(platforms) / sizeof(platforms[0]), platforms, &numPlatforms) != CL_SUCCESS)
        return "clGetPlatformIDs() failed";

    if (numPlatforms == 0)
        return "clGetPlatformIDs returned 0 platforms";

    char buffer[1024];
    for (cl_uint iplat = 0; iplat < numPlatforms; ++iplat)
    {
        if (data->ffclGetPlatformInfo(platforms[iplat], CL_PLATFORM_VERSION, sizeof(buffer), buffer, NULL) != CL_SUCCESS)
            return "clGetPlatformInfo() failed";

        // Use the newest supported OpenCL version
        if (ffStrbufCompS(&result->version, buffer) < 0)
        {
            const char* versionPretty = buffer;
            if(ffStrStartsWithIgnCase(buffer, "OpenCL "))
                versionPretty = buffer + strlen("OpenCL ");
            ffStrbufSetS(&result->version, versionPretty);
            ffStrbufTrim(&result->version, ' ');

            if (data->ffclGetPlatformInfo(platforms[iplat], CL_PLATFORM_NAME, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
                ffStrbufSetS(&result->name, buffer);

            if (data->ffclGetPlatformInfo(platforms[iplat], CL_PLATFORM_VENDOR, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
                ffStrbufSetS(&result->vendor, buffer);
        }

        cl_device_id deviceIDs[32];
        cl_uint numDevices = (cl_uint) (sizeof(deviceIDs) / sizeof(deviceIDs[0]));
        if (data->ffclGetDeviceIDs(platforms[iplat], CL_DEVICE_TYPE_GPU, numDevices, deviceIDs, &numDevices) != CL_SUCCESS)
            continue;

        for (cl_uint idev = 0; idev < numDevices; ++idev)
        {
            cl_device_id deviceID = deviceIDs[idev];
            if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_NAME, sizeof(buffer), buffer, NULL) != CL_SUCCESS)
                continue;

            FFGPUResult* gpu = ffListAdd(&result->gpus);
            ffStrbufInitS(&gpu->name, buffer);
            ffStrbufInit(&gpu->vendor);
            ffStrbufInit(&gpu->driver);
            ffStrbufInit(&gpu->platformApi);
            gpu->index = FF_GPU_INDEX_UNSET;
            gpu->temperature = FF_GPU_TEMP_UNSET;
            gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
            gpu->type = FF_GPU_TYPE_UNKNOWN;
            gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
            gpu->deviceId = (size_t) deviceID;
            gpu->frequency = FF_GPU_FREQUENCY_UNSET;
            gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;

            if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_VERSION, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
            {
                ffStrbufSetS(&gpu->platformApi, buffer);
                ffStrbufTrimRight(&gpu->platformApi, ' ');
            }
            else
                ffStrbufSetStatic(&gpu->platformApi, "OpenCL");

            {
                cl_uint vendorId;
                if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_VENDOR_ID, sizeof(vendorId), &vendorId, NULL) == CL_SUCCESS)
                    ffStrbufSetStatic(&gpu->vendor, ffGetGPUVendorString(vendorId));
                if (gpu->vendor.length == 0 && data->ffclGetDeviceInfo(deviceID, CL_DEVICE_VENDOR, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
                    ffStrbufSetS(&gpu->vendor, buffer);
            }

            if (data->ffclGetDeviceInfo(deviceID, CL_DRIVER_VERSION, sizeof(buffer), buffer, NULL) == CL_SUCCESS)
            {
                const char* versionPretty = strchr(buffer, ' ');
                if (versionPretty && *versionPretty)
                    ffStrbufSetS(&gpu->driver, versionPretty + 1);
                else
                    ffStrbufSetS(&gpu->driver, buffer);
            }

            {
                cl_uint value;
                if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(value), &value, NULL) == CL_SUCCESS)
                    gpu->coreCount = (int32_t) value;
            }

            {
                cl_uint value;
                if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(value), &value, NULL) == CL_SUCCESS)
                    gpu->frequency = value;
            }

            {
                cl_bool value;
                if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(value), &value, NULL) == CL_SUCCESS)
                {
                    gpu->type = value ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;

                    cl_ulong memSize;
                    if (data->ffclGetDeviceInfo(deviceID, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(memSize), &memSize, NULL) == CL_SUCCESS)
                    {
                        if (gpu->type == FF_GPU_TYPE_INTEGRATED)
                            gpu->shared.total = memSize;
                        else
                            gpu->dedicated.total = memSize;
                    }
                }
            }
        }
    }

    return NULL;
}

static const char* detectOpenCL(FFOpenCLResult* result)
{
    OpenCLData data;

    #ifndef __APPLE__

    FF_LIBRARY_LOAD(opencl, "dlopen libOpenCL" FF_LIBRARY_EXTENSION" failed",
    #ifdef _WIN32
        "OpenCL"FF_LIBRARY_EXTENSION, -1,
    #endif
        "libOpenCL"FF_LIBRARY_EXTENSION, 1
    );
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opencl, data, clGetPlatformIDs);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opencl, data, clGetPlatformInfo);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opencl, data, clGetDeviceIDs);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opencl, data, clGetDeviceInfo);

    return openCLHandleData(&data, result);

    #else

    data.ffclGetPlatformIDs = clGetPlatformIDs;
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
