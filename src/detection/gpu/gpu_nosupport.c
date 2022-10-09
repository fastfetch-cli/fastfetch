#include "gpu.h"

const char* ffDetectGPUImpl(FFlist* gpus, const FFinstance* instance)
{
    FF_UNUSED(gpus, instance);
    return "Not supported on this platform";
}
