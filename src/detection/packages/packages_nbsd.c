#include "packages.h"

#include "common/io.h"

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options) {
    if (FF_PACKAGES_IS_ENABLED(options, PKGSRC)) {
        result->pkgsrc = ffPackagesGetNumElements(FASTFETCH_TARGET_DIR_ROOT "/usr/pkg/pkgdb", true);
    }
}
