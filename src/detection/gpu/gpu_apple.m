#include "gpu.h"

#import <Metal/MTLDevice.h>

#ifndef MAC_OS_VERSION_13_0
    #define MTLGPUFamilyMetal3 ((MTLGPUFamily) 5001)
#endif

const char* ffGpuDetectMetal(FFlist* gpus)
{
    if (@available(macOS 10.13, *))
    {
        for (id<MTLDevice> device in MTLCopyAllDevices())
        {
            FFGPUResult* gpu = NULL;
            FF_LIST_FOR_EACH(FFGPUResult, x, *gpus)
            {
                if (x->deviceId == device.registryID)
                {
                    gpu = x;
                    break;
                }
            }
            if (!gpu) continue;

            #ifdef MAC_OS_X_VERSION_10_13
            #define MTLFeatureSet_macOS_GPUFamily2_v1 ((MTLFeatureSet) 10005)
            if ([device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily2_v1])
                ffStrbufSetStatic(&gpu->platformApi, "Metal Featrureset 2");
            else if ([device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v1])
                ffStrbufSetStatic(&gpu->platformApi, "Metal Featrureset 1");
            #endif

            #ifdef MAC_OS_X_VERSION_10_15
            if ([device supportsFamily:MTLGPUFamilyMetal3])
                ffStrbufSetStatic(&gpu->platformApi, "Metal 3");
            else if ([device supportsFamily:MTLGPUFamilyCommon3])
                ffStrbufSetStatic(&gpu->platformApi, "Metal Common 3");
            else if ([device supportsFamily:MTLGPUFamilyCommon2])
                ffStrbufSetStatic(&gpu->platformApi, "Metal Common 2");
            else if ([device supportsFamily:MTLGPUFamilyCommon1])
                ffStrbufSetStatic(&gpu->platformApi, "Metal Common 1");

            if (device.hasUnifiedMemory)
            {
                gpu->type = FF_GPU_TYPE_INTEGRATED;
                gpu->shared.total = device.recommendedMaxWorkingSetSize;
                gpu->shared.used = device.currentAllocatedSize;
            }
            else
            #endif
            {
                gpu->type = FF_GPU_TYPE_DISCRETE;
                gpu->dedicated.total = device.recommendedMaxWorkingSetSize;
                gpu->dedicated.used = device.currentAllocatedSize;
            }
        }
        return NULL;
    }
    return "Metal API is not supported by this macOS version";
}
