#include "libc.h"

const char* ffDetectLibc(FFLibcResult* result)
{
    result->name = "FBSD";

#ifdef FF_FBSD_VERSION
    result->version = FF_FBSD_VERSION;
#else
    result->version = NULL;
#endif
    return NULL;
}
