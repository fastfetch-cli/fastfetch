#include "packages.h"

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options) {
    if (FF_PACKAGES_IS_ENABLED(options, PKGSRC)) {
        FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateStatic(FASTFETCH_TARGET_DIR_ROOT);
        result->pkgsrc = ffPackagesGetPkgsrc(&baseDir);
    }
}
