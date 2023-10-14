#include "libc.h"

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

#include <features.h>

const char* ffDetectLibc(FFLibcResult* result)
{
#ifdef __UCLIBC__
    result->name = "uClibc";
    result->version = FF_STR(__UCLIBC_MAJOR__) "." FF_STR(__UCLIBC_MINOR__) "." FF_STR(__UCLIBC_SUBLEVEL__);
#elif defined(__GNU_LIBRARY__)
    result->name = "glibc";
    result->version = FF_STR(__GLIBC__) "." FF_STR(__GLIBC_MINOR__);
#else
    result->name = "musl";
    #ifdef FF_MUSL_VERSION
        result->version = FF_MUSL_VERSION;
    #else
        result->version = NULL;
    #endif
#endif

    return NULL;
}
