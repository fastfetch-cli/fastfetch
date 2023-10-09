#include "libc.h"

const char* ffDetectLibc(FFLibcResult* result)
{
    result->name = "libSystem";

#ifdef FF_LIBSYSTEM_VERSION
    result->version = FF_LIBSYSTEM_VERSION;
#else
    result->version = NULL;
#endif
    return NULL;
}
