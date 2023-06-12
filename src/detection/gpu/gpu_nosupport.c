#include "gpu.h"

const char* ffDetectGPUImpl(const FFinstance* instance, const FFGPUOptions* options, FFlist* gpus)
{
    FF_UNUSED(instance, options, gpus);
    return "Not supported on this platform";
}
