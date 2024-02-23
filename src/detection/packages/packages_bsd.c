#include "packages.h"

#include "common/settings.h"

void ffDetectPackagesImpl(FFPackagesResult* result, FF_MAYBE_UNUSED FFPackagesOptions* options)
{
    if (!(options->disabled & FF_PACKAGES_FLAG_PKG_BIT))
        result->pkg = (uint32_t) ffSettingsGetSQLite3Int(FASTFETCH_TARGET_DIR_ROOT "/var/db/pkg/local.sqlite", "SELECT count(*) FROM packages");
}
