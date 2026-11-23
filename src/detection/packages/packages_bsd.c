#include "packages.h"

#include "common/io.h"
#include "common/settings.h"

static uint32_t getSQLite3Int(const char* dbPath, const char* query, const char* packageId) {
    FF_STRBUF_AUTO_DESTROY cacheDir = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY cacheContent = ffStrbufCreate();

    uint32_t num_elements;
    if (ffPackagesReadCache(&cacheDir, &cacheContent, dbPath, packageId, &num_elements)) {
        return num_elements;
    }

    num_elements = (uint32_t) ffSettingsGetSQLite3Int(dbPath, query);

    ffPackagesWriteCache(&cacheDir, &cacheContent, num_elements);

    return num_elements;
}

static bool hasFlatpkgDeployment(FFstrbuf* path, uint32_t depth) {
    uint32_t length = path->length;
    if (depth == 0) {
        ffStrbufAppendS(path, "/active");
        bool active = ffPathExists(path->chars, FF_PATHTYPE_DIRECTORY);
        ffStrbufSubstrBefore(path, length);
        return active;
    }

    FF_AUTO_CLOSE_DIR DIR* dir = opendir(path->chars);
    if (dir == nullptr) {
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] == '.') {
            continue;
        }
        ffStrbufAppendC(path, '/');
        ffStrbufAppendS(path, entry->d_name);
        bool active = hasFlatpkgDeployment(path, depth - 1);
        ffStrbufSubstrBefore(path, length);
        if (active) {
            return true;
        }
    }

    return false;
}

static uint32_t countFlatpkgIds(FFstrbuf* path, const char* kind, bool isApp) {
    uint32_t rootLength = path->length;
    ffStrbufAppendS(path, kind);
    FF_AUTO_CLOSE_DIR DIR* dir = opendir(path->chars);
    uint32_t count = 0;
    if (dir != nullptr) {
        uint32_t kindLength = path->length;
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_name[0] == '.') {
                continue;
            }
            ffStrbufAppendS(path, entry->d_name);
            // Count each ID once, even if multiple architectures or branches are active.
            bool installed = hasFlatpkgDeployment(path, 2);
            if (!installed && isApp) {
                // An imported application can be represented by a pointer file.
                ffStrbufAppendS(path, "/pointer");
                struct stat info;
                installed = stat(path->chars, &info) == 0 && S_ISREG(info.st_mode);
            }
            count += installed;
            ffStrbufSubstrBefore(path, kindLength);
        }
    }
    ffStrbufSubstrBefore(path, rootLength);
    return count;
}

static uint32_t getFlatpkgPackages(FFstrbuf* store) {
    return countFlatpkgIds(store, "app/", true) + countFlatpkgIds(store, "runtime/", false) + countFlatpkgIds(store, "extension/", false);
}

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options) {
    if (FF_PACKAGES_IS_ENABLED(options, PKG)) {
        result->pkg = getSQLite3Int(FASTFETCH_TARGET_DIR_ROOT "/var/db/pkg/local.sqlite", "SELECT count(*) FROM packages", "pkg");
    }
    if (FF_PACKAGES_IS_ENABLED(options, MPORT)) {
        result->mport = getSQLite3Int(FASTFETCH_TARGET_DIR_ROOT "/var/db/mport/master.db", "SELECT count(*) FROM packages", "mport");
    }
    if (FF_PACKAGES_IS_ENABLED(options, FLATPAK)) {
        FF_STRBUF_AUTO_DESTROY systemDir = ffStrbufCreateStatic(FASTFETCH_TARGET_DIR_ROOT "/var/db/flatpkg/");
        result->flatpakSystem = getFlatpkgPackages(&systemDir);
        if (instance.state.platform.homeDir.length > 0) {
            FF_STRBUF_AUTO_DESTROY homeDir = ffStrbufCreateCopy(&instance.state.platform.homeDir);
            ffStrbufAppendS(&homeDir, ".local/share/flatpak/");
            result->flatpakUser = getFlatpkgPackages(&homeDir);
        }
    }
}
