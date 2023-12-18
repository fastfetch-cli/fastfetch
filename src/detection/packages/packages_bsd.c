#include "packages.h"

#include "common/settings.h"

void ffDetectPackagesImpl(FFPackagesResult* result, FF_MAYBE_UNUSED FFPackagesOptions* options)
{
    result->pkg = (uint32_t) ffSettingsGetSQLite3Int(FASTFETCH_TARGET_DIR_ROOT "/var/db/pkg/local.sqlite", "SELECT count(*) FROM packages");
}
