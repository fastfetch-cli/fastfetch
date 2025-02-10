#include "packages.h"

#include "common/io/io.h"

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options)
{
    // TODO: Use the Package Kit C++ API instead (would account for disabled packages)

    if (!(options->disabled & FF_PACKAGES_FLAG_PKG_BIT))
        result->pkg = ffPackagesGetNumElements(FASTFETCH_TARGET_DIR_ROOT "/system/packages", false);
    if (!(options->disabled & FF_PACKAGES_FLAG_PKGSRC_BIT))
        result->pkgsrc = ffPackagesGetNumElements(FASTFETCH_TARGET_DIR_ROOT "/boot/home/config/packages", false);
}
