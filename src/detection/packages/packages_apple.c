#include "packages.h"
#include "common/io/io.h"
#include "common/parsing.h"
#include "common/processing.h"
#include "util/FFstrbuf.h"
#include "util/stringUtils.h"
#include <stdint.h>

static void countBrewPackages(FFstrbuf* baseDir, FFPackagesResult* result)
{

    uint32_t baseDirLength = baseDir->length;

    ffStrbufAppendS(baseDir, "/Caskroom");
    result->brewCask += ffPackagesGetNumElements(baseDir->chars, true);
    ffStrbufSubstrBefore(baseDir, baseDirLength);

    ffStrbufAppendS(baseDir, "/Cellar");
    result->brew += ffPackagesGetNumElements(baseDir->chars, true);
    ffStrbufSubstrBefore(baseDir, baseDirLength);
}

static uint32_t getMacPortsPackages(FFstrbuf* baseDir)
{
    ffStrbufAppendS(baseDir, "/var/macports/software");
    return ffPackagesGetNumElements(baseDir->chars, true);
}

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options)
{
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreate();
    if (!(options->disabled & FF_PACKAGES_FLAG_BREW_BIT))
    {
        const char* prefix = getenv("HOMEBREW_PREFIX");
        if (ffStrSet(prefix))
        {
            ffStrbufSetS(&baseDir, prefix);
        }
        else
        {
            #ifdef __aarch64__
            ffStrbufSetS(&baseDir, FASTFETCH_TARGET_DIR_ROOT "/opt/homebrew");
            #else
            ffStrbufSetS(&baseDir, FASTFETCH_TARGET_DIR_USR "/local");
            #endif
        }
        countBrewPackages(&baseDir, result);
    }
    if (!(options->disabled & FF_PACKAGES_FLAG_MACPORTS_BIT))
    {
        const char* prefix = getenv("MACPORTS_PREFIX");
        if (ffStrSet(prefix))
        {
            ffStrbufSetS(&baseDir, prefix);
        }
        else
        {
            ffStrbufSetS(&baseDir, FASTFETCH_TARGET_DIR_ROOT "/opt/local");
        }

        result->macports = getMacPortsPackages(&baseDir);
    }
    if (!(options->disabled & FF_PACKAGES_FLAG_NIX_BIT))
    {
        ffStrbufSetS(&baseDir, FASTFETCH_TARGET_DIR_ROOT);
        ffStrbufAppendS(&baseDir, "/nix/var/nix/profiles/default");

        FF_STRBUF_AUTO_DESTROY dir2 = ffStrbufCreateS(FASTFETCH_TARGET_DIR_ROOT);
        ffStrbufAppendS(&dir2, "/run/current-system");

        FF_STRBUF_AUTO_DESTROY dir3 = ffStrbufCreateCopy(&instance.state.platform.homeDir);
        ffStrbufAppendS(&dir3, "/.nix-profile");

        FFstrbuf* dirs[] = {&baseDir, &dir2, &dir3};
        uint32_t counts[3] = {0};
        ffPackagesGetNixMulti(dirs, counts, 3);
        result->nixDefault = counts[0];
        result->nixSystem = counts[1];
        result->nixUser = counts[2];
    }
}
