#include "libc.h"
#include "common/strutil.h"

#include <features.h>

const char* ffDetectLibc(FFLibcResult* result) {
#if __ANDROID_NDK__
    result->name = "ndk-bionic";
    result->version = FF_STR(__NDK_MAJOR__) "." FF_STR(__NDK_MINOR__) "." FF_STR(__NDK_BUILD__)

    #if __NDK_BETA__
        "-beta" FF_STR(__NDK_BETA__)
    #elif __NDK_CANARY__
        "-canary"
    #endif

        ;
    return NULL;
#else
    return "Unknown Android libc";
#endif
}
