#include "packages.h"

#include "common/io/io.h"

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options)
{
    #if __OpenBSD__
    if (!(options->disabled & FF_PACKAGES_FLAG_PKG_BIT))
        result->pkg = ffPackagesGetNumElements(FASTFETCH_TARGET_DIR_ROOT "/var/db/pkg", true);
    #elif __NetBSD__
    if (!(options->disabled & FF_PACKAGES_FLAG_PKGSRC_BIT))
        result->pkgsrc = ffPackagesGetNumElements(FASTFETCH_TARGET_DIR_ROOT "/usr/pkg/pkgdb", true);
    #endif
}
