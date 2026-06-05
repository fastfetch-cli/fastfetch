#include "packages.h"

#include "common/io.h"

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options) {
    if (FF_PACKAGES_IS_ENABLED(options, PKG)) {
        result->pkg = ffPackagesGetNumElements(FASTFETCH_TARGET_DIR_ROOT "/var/db/pkg", true);
    }
}
