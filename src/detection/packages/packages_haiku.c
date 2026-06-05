#include "packages.h"

#include "common/io.h"

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options) {
    // TODO: Use the Package Kit C++ API instead (would account for disabled packages)

    if (FF_PACKAGES_IS_ENABLED(options, HPKG)) {
        result->hpkgSystem = ffPackagesGetNumElements(FASTFETCH_TARGET_DIR_ROOT "/system/packages", false);
        result->hpkgUser = ffPackagesGetNumElements(FASTFETCH_TARGET_DIR_ROOT "/boot/home/config/packages", false);
    }
}
