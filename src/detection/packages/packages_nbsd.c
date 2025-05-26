#include "packages.h"

#include "common/io/io.h"

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options)
{
    if (!(options->disabled & FF_PACKAGES_FLAG_PKGSRC_BIT))
        result->pkgsrc = ffPackagesGetNumElements(FASTFETCH_TARGET_DIR_ROOT "/usr/pkg/pkgdb", true);
}
