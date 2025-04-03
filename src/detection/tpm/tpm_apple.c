#include "tpm.h"

#ifndef __aarch64__
    #include "util/apple/cf_helpers.h"
    #include <IOKit/IOKitLib.h>
#endif

const char* ffDetectTPM(FFTPMResult* result)
{
    #ifdef __aarch64__

    ffStrbufSetStatic(&result->version, "2.0");
    ffStrbufSetStatic(&result->description, "Apple Silicon Security");
    return NULL;

    #else

    FF_IOOBJECT_AUTO_RELEASE io_service_t t2Service = IOServiceGetMatchingService(
        MACH_PORT_NULL,
        IOServiceMatching("AppleT2"));

    if (t2Service) {
        ffStrbufSetStatic(&result->version, "2.0");
        ffStrbufSetStatic(&result->description, "Apple T2 Security Chip");
        return NULL;
    }

    #endif

    return "No Apple Security hardware detected";
}
