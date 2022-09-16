#include "fastfetch.h"
#include "common/printing.h"

#define FF_OPENCL_MODULE_NAME "OpenCL"
#define FF_OPENCL_NUM_FORMAT_ARGS 3

#if defined(FF_HAVE_OPENCL) || defined(__APPLE__)
#include "common/library.h"
#include "common/parsing.h"
#include <string.h>

#define CL_TARGET_OPENCL_VERSION 100
#ifdef __APPLE__
    #include <OpenCL/cl.h>
#else
    #include <CL/cl.h>
#endif

typedef struct OpenCLData
{
    FF_LIBRARY_SYMBOL(clGetPlatformIDs);
    FF_LIBRARY_SYMBOL(clGetDeviceIDs);
    FF_LIBRARY_SYMBOL(clGetDeviceInfo);
} OpenCLData;

static const char* openCLHandelData(FFinstance* instance, OpenCLData* data)
{
    cl_platform_id platformID;
    cl_uint numPlatforms;
    data->ffclGetPlatformIDs(1, &platformID, &numPlatforms);

    if(numPlatforms == 0)
        return "clGetPlatformIDs returned 0 platforms";

    cl_device_id deviceID;
    cl_uint numDevices;
    data->ffclGetDeviceIDs(platformID, CL_DEVICE_TYPE_GPU, 1, &deviceID, &numDevices);

    if(numDevices == 0)
        data->ffclGetDeviceIDs(platformID, CL_DEVICE_TYPE_ALL, 1, &deviceID, &numDevices);

    if(numDevices == 0)
        return "clGetDeviceIDs returned 0 devices";

    char version[64];
    data->ffclGetDeviceInfo(deviceID, CL_DEVICE_VERSION, sizeof(version), version, NULL);
    if(!ffStrSet(version))
        return "clGetDeviceInfo returned NULL or empty string";

    const char* versionPretty = version;
    const char* prefix = "OpenCL ";
    if(strncasecmp(version, prefix, sizeof(prefix) - 1) == 0)
        versionPretty = version + sizeof(prefix) - 1;

    char device[128];
    data->ffclGetDeviceInfo(deviceID, CL_DEVICE_NAME, sizeof(device), device, NULL);

    char vendor[32];
    data->ffclGetDeviceInfo(deviceID, CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL);

    if(instance->config.openCL.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_OPENCL_MODULE_NAME, 0, &instance->config.openCL.key);
        puts(versionPretty);
    }
    else
    {
        ffPrintFormat(instance, FF_OPENCL_MODULE_NAME, 0, &instance->config.openCL, FF_OPENCL_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRING, versionPretty},
            {FF_FORMAT_ARG_TYPE_STRING, device},
            {FF_FORMAT_ARG_TYPE_STRING, vendor}
        });
    }

    return NULL;
}

#endif // FF_HAVE_OPENCL || __APPLE__

#ifdef FF_HAVE_OPENCL

static const char* printOpenCL(FFinstance* instance)
{
    OpenCLData data;

    FF_LIBRARY_LOAD(opencl, instance->config.libOpenCL, "dlopen libOpenCL.so failed", "libOpenCL.so", 1);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opencl, data, clGetPlatformIDs);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opencl, data, clGetDeviceIDs);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opencl, data, clGetDeviceInfo);

    const char* error = openCLHandelData(instance, &data);
    dlclose(opencl);
    return error;
}

#elif __APPLE__ // FF_HAVE_OPENCL

static const char* printOpenCL(FFinstance* instance)
{
    OpenCLData data;
    data.ffclGetPlatformIDs = clGetPlatformIDs;
    data.ffclGetDeviceIDs = clGetDeviceIDs;
    data.ffclGetDeviceInfo = clGetDeviceInfo;

    return openCLHandelData(instance, &data);
}

#endif // __APPLE__

void ffPrintOpenCL(FFinstance* instance)
{
    const char* error;

    #if defined(FF_HAVE_OPENCL) || defined(__APPLE__)
        error = printOpenCL(instance);
    #else
        error = "Fastfetch was build without OpenCL support";
    #endif

    if(error != NULL)
        ffPrintError(instance, FF_OPENCL_MODULE_NAME, 0, &instance->config.openCL, "%s", error);
}
