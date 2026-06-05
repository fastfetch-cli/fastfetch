#include "packages.h"
#include <dirent.h>

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options) {
    if (FF_PACKAGES_IS_ENABLED(options, PKG)) {
        yyjson_doc* doc = yyjson_read_file(FASTFETCH_TARGET_DIR_ROOT "/var/pkg/state/installed/catalog.attrs", YYJSON_READ_NOFLAG, NULL, NULL);
        if (doc) {
            yyjson_val* packageCount = yyjson_obj_get(yyjson_doc_get_root(doc), "package-count");
            if (packageCount) {
                result->pkg = (uint32_t) yyjson_get_uint(packageCount);
            }
        }
    }
    if (FF_PACKAGES_IS_ENABLED(options, PKGSRC)) {
        result->pkgsrc = ffPackagesGetNumElements(FASTFETCH_TARGET_DIR_ROOT "/usr/pkg/pkgdb", true);
    }
}
