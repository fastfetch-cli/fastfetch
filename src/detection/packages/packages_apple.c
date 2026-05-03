#include "packages.h"
#include "common/io.h"
#include "common/parsing.h"
#include "common/processing.h"
#include "common/stringUtils.h"

static void countBrewPackages(FFstrbuf* baseDir, FFPackagesResult* result) {
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, "/Caskroom");
    result->brewCask += ffPackagesGetNumElements(baseDir->chars, true);
    ffStrbufSubstrBefore(baseDir, baseDirLength);
    ffStrbufAppendS(baseDir, "/Cellar");
    result->brew += ffPackagesGetNumElements(baseDir->chars, true);
    ffStrbufSubstrBefore(baseDir, baseDirLength);
}

static uint32_t getMacPortsPackages(FFstrbuf* baseDir) {
    ffStrbufAppendS(baseDir, "/var/macports/software");
    return ffPackagesGetNumElements(baseDir->chars, true);
}

static uint32_t getDpkgPackages(FFstrbuf* baseDir) {
    ffStrbufSetS(baseDir, "/opt/procursus/var/lib/dpkg/status");

    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    if (!ffReadFileBuffer(baseDir->chars, &content)) {
        return 0;
    }

    const char* needle = "Status: install ok installed";
    size_t needleLength = strlen(needle);
    uint32_t count = 0;
    char* iter = content.chars;
    while ((iter = memmem(iter, content.length - (size_t)(iter - content.chars), needle, needleLength)) != NULL) {
        ++count;
        iter += needleLength;
    }
    return count;
}

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options) {
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreate();

    if (!(options->disabled & FF_PACKAGES_FLAG_BREW_BIT)) {
        const char* prefix = getenv("HOMEBREW_PREFIX");
        if (ffStrSet(prefix)) {
            ffStrbufSetS(&baseDir, prefix);
        } else {
#ifdef __aarch64__
            ffStrbufSetS(&baseDir, FASTFETCH_TARGET_DIR_ROOT "/opt/homebrew");
#else
            ffStrbufSetS(&baseDir, FASTFETCH_TARGET_DIR_USR "/local");
#endif
        }
        countBrewPackages(&baseDir, result);
    }

    if (!(options->disabled & FF_PACKAGES_FLAG_MACPORTS_BIT)) {
        const char* prefix = getenv("MACPORTS_PREFIX");
        if (ffStrSet(prefix)) {
            ffStrbufSetS(&baseDir, prefix);
        } else {
            ffStrbufSetS(&baseDir, FASTFETCH_TARGET_DIR_ROOT "/opt/local");
        }
        result->macports = getMacPortsPackages(&baseDir);
    }

    if (!(options->disabled & FF_PACKAGES_FLAG_DPKG_BIT)) {
        result->dpkg += getDpkgPackages(&baseDir);
    }

    if (!(options->disabled & FF_PACKAGES_FLAG_NIX_BIT)) {
        ffStrbufSetS(&baseDir, FASTFETCH_TARGET_DIR_ROOT);
        result->nixDefault += ffPackagesGetNix(&baseDir, "/nix/var/nix/profiles/default");
        result->nixSystem += ffPackagesGetNix(&baseDir, "/run/current-system");
        ffStrbufSet(&baseDir, &instance.state.platform.homeDir);
        result->nixUser = ffPackagesGetNix(&baseDir, "/.nix-profile");
    }
}
