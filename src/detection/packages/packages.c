#include "packages.h"
#include "detection/internal.h"

void ffDetectPackagesImpl(const FFinstance* instance, FFPackagesResult* result);

const FFPackagesResult* ffDetectPackages(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFPackagesResult,
        memset(&result, 0, sizeof(FFPackagesResult));
        ffStrbufInit(&result.pacmanBranch);

        ffDetectPackagesImpl(instance, &result);

        result.all = 0
            + result.pacman
            + result.dpkg
            + result.rpm
            + result.emerge
            + result.xbps
            + result.nixSystem
            + result.nixUser
            + result.nixDefault
            + result.apk
            + result.pkg
            + result.flatpak
            + result.snap
            + result.brew
            + result.port
            + result.scoop;
    );
}
