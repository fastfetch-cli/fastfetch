#include "modules/gpu/gpu.h"

#include <stdlib.h>

static void verify(const char* vendor, const char* name, const char* expected, int lineNo) {
    // Supply detected data directly so output tests do not depend on installed GPU drivers.
    FFGPUResult gpu = {};
    ffStrbufInitS(&gpu.vendor, vendor);
    ffStrbufInitS(&gpu.name, name);
    FF_STRBUF_AUTO_DESTROY output = ffStrbufCreate();
    ffGPUAppendName(&gpu, &output);
    if (!ffStrbufEqualS(&output, expected)) {
        fprintf(stderr, "[%d] Expected \"%s\", got \"%s\"\n", lineNo, expected, output.chars);
        exit(1);
    }

    // Rendering must not change the detected values used in JSON and custom formats.
    if (!ffStrbufEqualS(&gpu.vendor, vendor) || !ffStrbufEqualS(&gpu.name, name)) {
        fprintf(stderr, "[%d] Detected GPU data changed during rendering\n", lineNo);
        exit(1);
    }
    ffStrbufDestroy(&gpu.vendor);
    ffStrbufDestroy(&gpu.name);
}

#define VERIFY(vendor, name, expected) verify((vendor), (name), (expected), __LINE__)

int main(void) {
    VERIFY("NVIDIA", "Microsoft Basic Display Adapter", "Microsoft Basic Display Adapter");
    VERIFY("AMD", "Microsoft Basic Display Adapter", "Microsoft Basic Display Adapter");
    VERIFY("Intel", "Microsoft Basic Display Adapter", "Microsoft Basic Display Adapter");
    VERIFY("", "Microsoft Basic Display Adapter", "Microsoft Basic Display Adapter");
    VERIFY("Microsoft", "Microsoft Basic Display Adapter", "Microsoft Basic Display Adapter");
    VERIFY("NVIDIA", "GeForce RTX 4090", "NVIDIA GeForce RTX 4090");
    VERIFY("NVIDIA", "NVIDIA GeForce RTX 4090", "NVIDIA GeForce RTX 4090");
    VERIFY("NVIDIA", "nvidia GeForce RTX 4090", "nvidia GeForce RTX 4090");
    VERIFY("", "GeForce RTX 4090", "GeForce RTX 4090");
    return 0;
}
