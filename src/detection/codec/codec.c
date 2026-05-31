#include "codec.h"

const char* ffDetectCodec(FFCodecOptions* options, FFlist* result /*list of FFCodecResult*/) {
    if (options->useVulkan) {
#ifdef FF_HAVE_VULKAN
        return ffDetectCodecVulkan(result);
#else
        FF_UNUSED(result);
        return "Fastfetch was built without Vulkan support";
#endif
    }

    return ffDetectCodecNative(options, result);
}
