#include "chassis.h"
#include "common/debug.h"
#include "detection/host/host.h"

const char* ffDetectChassis(FFChassisResult* result) {
    FFHostResult host = {
        .family = ffStrbufCreate(),
        .name = ffStrbufCreate(),
        .version = ffStrbufCreate(),
        .sku = ffStrbufCreate(),
        .serial = ffStrbufCreate(),
        .uuid = ffStrbufCreate(),
        .vendor = ffStrbufCreate(),
    };
    const char* hostError = ffDetectHost(&host);
    if (hostError != nullptr) {
        FF_DEBUG("ffDetectHost() failed: %s", hostError);
        return "Failed to detect host";
    }

    if (ffStrbufStartsWithS(&host.name, "MacBook ")) {
        ffStrbufSetStatic(&result->type, "Laptop");
    } else if (ffStrbufStartsWithS(&host.name, "Mac mini ") ||
        ffStrbufStartsWithS(&host.name, "Mac Studio ")) {
        ffStrbufSetStatic(&result->type, "Mini PC");
    } else if (ffStrbufStartsWithS(&host.name, "iMac ")) {
        ffStrbufSetStatic(&result->type, "All in One");
    } else {
        ffStrbufSetStatic(&result->type, "Desktop");
    }

    ffStrbufSet(&result->vendor, &host.vendor);

    ffStrbufDestroy(&host.family);
    ffStrbufDestroy(&host.name);
    ffStrbufDestroy(&host.version);
    ffStrbufDestroy(&host.sku);
    ffStrbufDestroy(&host.serial);
    ffStrbufDestroy(&host.uuid);
    ffStrbufDestroy(&host.vendor);

    return nullptr;
}
