#include "libc.h"

const char* ffDetectLibc(FFLibcResult* result)
{
    result->name = "Unknown";
    result->version = NULL;

#ifdef __DragonFly__ // We define `__FreeBSD__` on DragonFly BSD for simplification
    result->name = "DF";
    #ifdef FF_DF_VERSION
    result->version = FF_DF_VERSION;
    #endif
#elif __FreeBSD__
    result->name = "FBSD";
    #ifdef FF_FBSD_VERSION
    result->version = FF_FBSD_VERSION;
    #endif
#endif

    return NULL;
}
