#include "gpu.h"

const char* ffDetectGPUImpl(const FFGPUOptions* options, FFlist* gpus)
{
    FF_UNUSED(options, gpus);
    return "Not supported on this platform";
}
