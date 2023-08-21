#include "version.h"

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64)
    #define FF_ARCHITECTURE "x86_64"
#elif defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(__i586__) || defined(__i586) || defined(__i686__) || defined(__i686)
    #define FF_ARCHITECTURE "i386"
#elif defined(__aarch64__)
    #define FF_ARCHITECTURE "aarch64"
#elif defined(__arm__)
    #define FF_ARCHITECTURE "arm"
#elif defined(__mips__)
    #define FF_ARCHITECTURE "mips"
#elif defined(__powerpc__) || defined(__powerpc)
    #define FF_ARCHITECTURE "powerpc"
#elif defined(__riscv__) || defined(__riscv)
    #define FF_ARCHITECTURE "riscv"
#elif defined(__s390x__)
    #define FF_ARCHITECTURE "s390x"
#else
    #define FF_ARCHITECTURE "unknown"
#endif

void ffDetectVersion(FFVersionResult* version)
{
    version->projectName = FASTFETCH_PROJECT_NAME;
    version->architecture = FF_ARCHITECTURE;
    version->version = FASTFETCH_PROJECT_VERSION;
    version->versionTweak = FASTFETCH_PROJECT_VERSION_TWEAK;
    version->cmakeBuiltType = FASTFETCH_PROJECT_CMAKE_BUILD_TYPE;
    #ifndef NDEBUG
        version->debugMode = true;
    #else
        version->debugMode = false;
    #endif
}
