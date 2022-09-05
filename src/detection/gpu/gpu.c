#include "gpu.h"
#include "detection/internal.h"

void ffDetectGPUImpl(FFlist* gpus, const FFinstance* instance);

const FFlist* ffDetectGPU(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFlist,
        ffListInit(&result, sizeof(FFGPUResult));
        ffDetectGPUImpl(&result, instance);
    );
}
