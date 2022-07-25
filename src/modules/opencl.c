#include "fastfetch.h"
#include "common/printing.h"

#define FF_OPENCL_MODULE_NAME "OpenCL"
#define FF_OPENCL_NUM_FORMAT_ARGS 3

#ifdef FF_HAVE_OPENCL
#include "common/library.h"

#define CL_TARGET_OPENCL_VERSION 100
#include <CL/cl.h>

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

    char device[128];
    data->ffclGetDeviceInfo(deviceID, CL_DEVICE_NAME, sizeof(device), device, NULL);

    char vendor[32];
    data->ffclGetDeviceInfo(deviceID, CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL);

    if(instance->config.openCL.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_OPENCL_MODULE_NAME, 0, &instance->config.openCL.key);
        puts(version);
    }
    else
    {
        ffPrintFormat(instance, FF_OPENCL_MODULE_NAME, 0, &instance->config.openCL, FF_OPENCL_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRING, version},
            {FF_FORMAT_ARG_TYPE_STRING, device},
            {FF_FORMAT_ARG_TYPE_STRING, vendor}
        });
    }

    return NULL;
}

static const char* printOpenCL(FFinstance* instance)
{
    OpenCLData data;

    FF_LIBRARY_LOAD(opencl, instance->config.libOpenCL, "dlopen libOpenCL.so failed", "libOpenCL.so", 1);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(opencl, data.ffclGetPlatformIDs, clGetPlatformIDs, "dlsym clGetPlatformIDs failed");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(opencl, data.ffclGetDeviceIDs, clGetDeviceIDs, "dlsym clGetDeviceIDs failed");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(opencl, data.ffclGetDeviceInfo, clGetDeviceInfo, "dlsym clGetDeviceInfo failed");

    const char* error = openCLHandelData(instance, &data);
    dlclose(opencl);
    return error;
}
#endif

void ffPrintOpenCL(FFinstance* instance)
{
    const char* error;

    #ifdef FF_HAVE_OPENCL
        error = printOpenCL(instance);
    #else
        error = "Fastfetch was build without OpenCL support";
    #endif

    if(error != NULL)
        ffPrintError(instance, FF_OPENCL_MODULE_NAME, 0, &instance->config.openCL, error);
}
