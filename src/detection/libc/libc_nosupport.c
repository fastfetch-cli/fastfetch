#include "libc.h"

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

const char* ffDetectLibc(FF_MAYBE_UNUSED FFLibcResult* result)
{
    return "Not supported on this platform";
}
