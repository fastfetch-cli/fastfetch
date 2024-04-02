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

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

void ffDetectVersion(FFVersionResult* version)
{
    version->projectName = FASTFETCH_PROJECT_NAME;
    version->architecture = FF_ARCHITECTURE;
    version->version = FASTFETCH_PROJECT_VERSION;
    version->versionTweak = FASTFETCH_PROJECT_VERSION_TWEAK;
    version->cmakeBuiltType = FASTFETCH_PROJECT_CMAKE_BUILD_TYPE;
    version->compileTime = __DATE__ ", " __TIME__;
    #ifdef __clang__
        version->compiler =
            #ifdef _MSC_VER
                "clang-cl " ;
            #elif defined(__APPLE__) && defined(__apple_build_version__)
                "Apple clang "
            #else
                "clang "
            #endif

            FF_STR(__clang_major__) "." FF_STR(__clang_minor__) "." FF_STR(__clang_patchlevel__)

            #if defined(__APPLE__) && defined(__apple_build_version__)
                " (" FF_STR(__apple_build_version__) ")"
            #endif
            ;
    #elif defined(__GNUC__)
        version->compiler = "gcc " FF_STR(__GNUC__) "." FF_STR(__GNUC_MINOR__) "." FF_STR(__GNUC_PATCHLEVEL__);
    #elif defined(_MSC_VER)
        version->compiler = "msvc " FF_STR(_MSC_VER);
    #else
        version->compiler = "unknown";
    #endif
    #ifndef NDEBUG
        version->debugMode = true;
    #else
        version->debugMode = false;
    #endif
}
