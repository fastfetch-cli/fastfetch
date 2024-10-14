#include "packages.h"

#include "common/settings.h"

static uint32_t getSQLite3Int(const char* dbPath, const char* query, const char* packageId)
{
    FF_STRBUF_AUTO_DESTROY cacheDir = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY cacheContent = ffStrbufCreate();

    uint32_t num_elements;
    if (ffPackagesReadCache(&cacheDir, &cacheContent, dbPath, packageId, &num_elements))
        return num_elements;

    num_elements = (uint32_t) ffSettingsGetSQLite3Int(dbPath, query);

    ffPackagesWriteCache(&cacheDir, &cacheContent, num_elements);

    return num_elements;
}

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options)
{
    if (!(options->disabled & FF_PACKAGES_FLAG_PKG_BIT))
    {
        result->pkg = getSQLite3Int(FASTFETCH_TARGET_DIR_ROOT "/var/db/pkg/local.sqlite", "SELECT count(*) FROM packages", "pkg");
    }
}
