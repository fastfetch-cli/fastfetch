#include "detection/opencl/opencl.h"

#ifdef __APPLE__
#include <AvailabilityMacros.h>
#if (MAC_OS_X_VERSION_MIN_REQUIRED > 1050) && !defined(__ppc__)
#define MACOS_HAS_OPENCL
#endif
#endif

#if defined(FF_HAVE_OPENCL) || defined(MACOS_HAS_OPENCL)

#include "common/library.h"
#include "common/parsing.h"
#include "util/stringUtils.h"
#include <string.h>

#define CL_TARGET_OPENCL_VERSION 100
#ifdef FF_HAVE_OPENCL
    #include <CL/cl.h>
#else
    #include <OpenCL/cl.h>
#endif

typedef struct OpenCLData
{
    FF_LIBRARY_SYMBOL(clGetPlatformIDs)
    FF_LIBRARY_SYMBOL(clGetDeviceIDs)
    FF_LIBRARY_SYMBOL(clGetDeviceInfo)
} OpenCLData;

static const char* openCLHandleData(OpenCLData* data, FFOpenCLResult* result)
{
    cl_platform_id platformID = NULL;
    cl_uint numPlatforms = 0;
    data->ffclGetPlatformIDs(1, &platformID, &numPlatforms);

    if(numPlatforms == 0)
        return "clGetPlatformIDs returned 0 platforms";

    cl_device_id deviceID = NULL;
    cl_uint numDevices = 0;
    data->ffclGetDeviceIDs(platformID, CL_DEVICE_TYPE_GPU, 1, &deviceID, &numDevices);

    if(numDevices == 0)
        data->ffclGetDeviceIDs(platformID, CL_DEVICE_TYPE_ALL, 1, &deviceID, &numDevices);

    if(numDevices == 0)
        return "clGetDeviceIDs returned 0 devices";

    char version[64] = "";
    data->ffclGetDeviceInfo(deviceID, CL_DEVICE_VERSION, sizeof(version), version, NULL);
    if(!ffStrSet(version))
        return "clGetDeviceInfo returned NULL or empty string";

    const char* versionPretty = version;
    if(ffStrStartsWithIgnCase(version, "OpenCL "))
        versionPretty = version + strlen("OpenCL ");
    ffStrbufSetS(&result->version, versionPretty);
    ffStrbufTrim(&result->version, ' ');

    ffStrbufEnsureFree(&result->device, 128);
    data->ffclGetDeviceInfo(deviceID, CL_DEVICE_NAME, result->device.allocated, result->device.chars, NULL);
    ffStrbufRecalculateLength(&result->device);
    ffStrbufTrim(&result->device, ' ');

    ffStrbufEnsureFree(&result->vendor, 32);
    data->ffclGetDeviceInfo(deviceID, CL_DEVICE_VENDOR, result->vendor.allocated, result->vendor.chars, NULL);
    ffStrbufRecalculateLength(&result->vendor);
    ffStrbufTrim(&result->vendor, ' ');

    return NULL;
}

#endif // defined(FF_HAVE_OPENCL) || defined(MACOS_HAS_OPENCL)

const char* ffDetectOpenCL(FFOpenCLResult* result)
{
    #ifdef FF_HAVE_OPENCL

    OpenCLData data;

    FF_LIBRARY_LOAD(opencl, &instance.config.library.libOpenCL, "dlopen libOpenCL"FF_LIBRARY_EXTENSION" failed",
    #ifdef _WIN32
        "OpenCL"FF_LIBRARY_EXTENSION, -1,
    #endif
        "libOpenCL"FF_LIBRARY_EXTENSION, 1
    );
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opencl, data, clGetPlatformIDs);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opencl, data, clGetDeviceIDs);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opencl, data, clGetDeviceInfo);

    return openCLHandleData(&data, result);

    #elif defined(MACOS_HAS_OPENCL) // FF_HAVE_OPENCL

    OpenCLData data;
    data.ffclGetPlatformIDs = clGetPlatformIDs;
    data.ffclGetDeviceIDs = clGetDeviceIDs;
    data.ffclGetDeviceInfo = clGetDeviceInfo;

    return openCLHandleData(&data, result);

    #else

    FF_UNUSED(result);
    return "Fastfetch was build without OpenCL support";

    #endif // FF_HAVE_OPENCL
}
