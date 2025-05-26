#include "packages.h"

#include "common/io/io.h"

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options)
{
    if (!(options->disabled & FF_PACKAGES_FLAG_PKG_BIT))
        result->pkg = ffPackagesGetNumElements(FASTFETCH_TARGET_DIR_ROOT "/var/db/pkg", true);
}
