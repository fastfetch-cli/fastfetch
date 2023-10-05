#include "libc.h"

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

#include <features.h>

const char* ffDetectLibc(FFLibcResult* result)
{
#ifdef __GNU_LIBRARY__
    result->name = "glibc";
    result->version = FF_STR(__GLIBC__) "." FF_STR(__GLIBC_MINOR__);
#else
    result->name = "musl";
    result->version = NULL;
#endif

    return NULL;
}
