#include "os.h"
#include "common/properties.h"
#include "common/strutil.h"

void ffDetectOSImpl(FFOSResult* result) {
    ffParsePropFileValues(
#ifdef FF_CUSTOM_OS_RELEASE_PATH
        FF_STR(FF_CUSTOM_OS_RELEASE_PATH)
#else
        "/etc/os-release"
#endif
            ,
        11,
        (FFpropquery[]){
            { "PRETTY_NAME =", &result->prettyName },
            { "NAME =", &result->name },
            { "ID =", &result->id },
            { "ID_LIKE =", &result->idLike },
            { "VARIANT =", &result->variant },
            { "VARIANT_ID =", &result->variantID },
            { "VERSION =", &result->version },
            { "VERSION_ID =", &result->versionID },
            { "VERSION_CODENAME =", &result->codename },
            { "CODENAME =", &result->codename },
            { "BUILD_ID =", &result->buildID },
        });
}
