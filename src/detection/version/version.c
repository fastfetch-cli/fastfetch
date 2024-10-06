#include "version.h"

#if defined(__x86_64__)
    #define FF_ARCHITECTURE "x86_64"
#elif defined(__i386__)
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
#elif defined(__loongarch__)
    #define FF_ARCHITECTURE "loongarch"
#else
    #define FF_ARCHITECTURE "unknown"
#endif

#if defined(__ANDROID__)
    #define FF_SYSNAME "Android"
#elif defined(__linux__)
    #define FF_SYSNAME "Linux"
#elif defined(__FreeBSD__)
    #define FF_SYSNAME "FreeBSD"
#elif defined(__APPLE__)
    #define FF_SYSNAME "Darwin"
#elif defined(_WIN32)
    #define FF_SYSNAME "WIN32"
#elif defined(__sun)
    #define FF_SYSNAME "SunOS"
#elif defined(__OpenBSD__)
    #define FF_SYSNAME "OpenBSD"
#else
    #define FF_SYSNAME "unknown"
#endif

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

FFVersionResult ffVersionResult = {
    .projectName = FASTFETCH_PROJECT_NAME,
    .sysName = FF_SYSNAME,
    .architecture = FF_ARCHITECTURE,
    .version = FASTFETCH_PROJECT_VERSION,
    .versionTweak = FASTFETCH_PROJECT_VERSION_TWEAK,
    .versionGit = FASTFETCH_PROJECT_VERSION_GIT,
    .cmakeBuiltType = FASTFETCH_PROJECT_CMAKE_BUILD_TYPE,
    .compileTime = __DATE__ ", " __TIME__,
    .compiler =

    #ifdef __clang__
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
        ,
    #elif defined(__GNUC__)
        "gcc " FF_STR(__GNUC__) "." FF_STR(__GNUC_MINOR__) "." FF_STR(__GNUC_PATCHLEVEL__),
    #elif defined(_MSC_VER)
        "msvc " FF_STR(_MSC_VER),
    #else
        "unknown",
    #endif

    .debugMode =
    #ifndef NDEBUG
        true,
    #else
        false,
    #endif
};
