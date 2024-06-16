#include "packages.h"

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options)
{
    if (!(options->disabled & FF_PACKAGES_FLAG_PKG_BIT))
    {
        yyjson_doc* doc = yyjson_read_file(FASTFETCH_TARGET_DIR_ROOT "/var/pkg/state/installed/catalog.attrs", YYJSON_READ_NOFLAG, NULL, NULL);
        if (doc)
        {
            yyjson_val* packageCount = yyjson_obj_get(yyjson_doc_get_root(doc), "package-count");
            if (packageCount)
                result->pkg = (uint32_t) yyjson_get_uint(packageCount);
        }
    }
}
