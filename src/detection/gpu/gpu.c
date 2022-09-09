#include "gpu.h"
#include "detection/internal.h"
#include "detection/vulkan.h"

void ffDetectGPUImpl(FFlist* gpus, const FFinstance* instance);

const FFlist* ffDetectGPU(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFlist,
        ffListInit(&result, sizeof(FFGPUResult));
        ffDetectGPUImpl(&result, instance);

        if(result.length == 0)
        {
            const FFVulkanResult* vulkan = ffDetectVulkan(instance);
            result = vulkan->gpus;
        }
    );
}
