#include "packages.h"
#include "common/io/io.h"
#include "common/parsing.h"
#include "common/properties.h"
#include "common/settings.h"
#include "detection/os/os.h"
#include "util/stringUtils.h"

static uint32_t getNumElements(FFstrbuf* baseDir, const char* dirname, bool isdir)
{
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, dirname);
    uint32_t num_elements = ffPackagesGetNumElements(baseDir->chars, isdir);
    ffStrbufSubstrBefore(baseDir, baseDirLength);
    return num_elements;
}

static uint32_t getNumStringsImpl(const char* filename, const char* needle)
{
    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    if (!ffReadFileBuffer(filename, &content))
        return 0;

    uint32_t count = 0;
    char *iter = content.chars;
    size_t needleLength = strlen(needle);
    while ((iter = memmem(iter, content.length - (size_t)(iter - content.chars), needle, needleLength)) != NULL)
    {
        ++count;
        iter += needleLength;
    }

    return count;
}

static uint32_t getNumStrings(FFstrbuf* baseDir, const char* filename, const char* needle, const char* packageId)
{
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, filename);

    FF_STRBUF_AUTO_DESTROY cacheDir = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY cacheContent = ffStrbufCreate();

    uint32_t num_elements;
    if (ffPackagesReadCache(&cacheDir, &cacheContent, baseDir->chars, packageId, &num_elements))
    {
        ffStrbufSubstrBefore(baseDir, baseDirLength);
        return num_elements;
    }

    num_elements = getNumStringsImpl(baseDir->chars, needle);
    ffStrbufSubstrBefore(baseDir, baseDirLength);

    ffPackagesWriteCache(&cacheDir, &cacheContent, num_elements);

    return num_elements;
}

static uint32_t getSQLite3Int(FFstrbuf* baseDir, const char* dbPath, const char* query, const char* packageId)
{
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, dbPath);

    FF_STRBUF_AUTO_DESTROY cacheDir = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY cacheContent = ffStrbufCreate();

    uint32_t num_elements;
    if (ffPackagesReadCache(&cacheDir, &cacheContent, baseDir->chars, packageId, &num_elements))
    {
        ffStrbufSubstrBefore(baseDir, baseDirLength);
        return num_elements;
    }

    num_elements = (uint32_t) ffSettingsGetSQLite3Int(baseDir->chars, query);
    ffStrbufSubstrBefore(baseDir, baseDirLength);

    ffPackagesWriteCache(&cacheDir, &cacheContent, num_elements);

    return num_elements;
}

static uint32_t countFilesRecursiveImpl(FFstrbuf* baseDirPath, const char* filename)
{
    uint32_t baseDirPathLength = baseDirPath->length;

    ffStrbufAppendC(baseDirPath, '/');
    ffStrbufAppendS(baseDirPath, filename);
    bool exists = ffPathExists(baseDirPath->chars, FF_PATHTYPE_FILE);
    ffStrbufSubstrBefore(baseDirPath, baseDirPathLength);
    if(exists)
        return 1;

    DIR* dirp = opendir(baseDirPath->chars);
    if(dirp == NULL)
        return 0;

    ffStrbufAppendC(baseDirPath, '/');
    baseDirPathLength = baseDirPath->length;

    uint32_t sum = 0;

    struct dirent *entry;
    while((entry = readdir(dirp)) != NULL) {
        // According to the PMS, neither category nor package name can begin with '.', so no need to check for . or .. specifically
        if(entry->d_type != DT_DIR || entry->d_name[0] == '.')
            continue;

        ffStrbufAppendS(baseDirPath, entry->d_name);
        sum += countFilesRecursiveImpl(baseDirPath, filename);
        ffStrbufSubstrBefore(baseDirPath, baseDirPathLength);
    }

    closedir(dirp);
    return sum;
}

static uint32_t countFilesRecursive(FFstrbuf* baseDir, const char* dirname, const char* filename)
{
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, dirname);
    uint32_t sum = countFilesRecursiveImpl(baseDir, filename);
    ffStrbufSubstrBefore(baseDir, baseDirLength);
    return sum;
}

static uint32_t getXBPSImpl(FFstrbuf* baseDir)
{
    DIR* dir = opendir(baseDir->chars);
    if(dir == NULL)
        return 0;

    uint32_t result = 0;

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_type != DT_REG || !ffStrStartsWithIgnCase(entry->d_name, "pkgdb-"))
            continue;

        ffStrbufAppendC(baseDir, '/');
        ffStrbufAppendS(baseDir, entry->d_name);
        result = getNumStringsImpl(baseDir->chars, "<string>installed</string>");
        break;
    }

    closedir(dir);
    return result;
}

static uint32_t getXBPS(FFstrbuf* baseDir, const char* dirname)
{
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, dirname);
    uint32_t result = getXBPSImpl(baseDir);
    ffStrbufSubstrBefore(baseDir, baseDirLength);
    return result;
}

static uint32_t getSnap(FFstrbuf* baseDir)
{
    uint32_t result = getNumElements(baseDir, "/snap", true);

    if (result == 0)
        result = getNumElements(baseDir, "/var/lib/snapd/snap", true);

    //Accounting for the /snap/bin folder
    return result > 0 ? result - 1 : 0;
}

#ifdef FF_HAVE_RPM
#include "common/library.h"
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlog.h>

static uint32_t getRpmFromLibrpm(void)
{
    FF_LIBRARY_LOAD(rpm, 0, "librpm" FF_LIBRARY_EXTENSION, 12)
    FF_LIBRARY_LOAD_SYMBOL(rpm, rpmReadConfigFiles, 0)
    FF_LIBRARY_LOAD_SYMBOL(rpm, rpmtsCreate, 0)
    FF_LIBRARY_LOAD_SYMBOL(rpm, rpmtsInitIterator, 0)
    FF_LIBRARY_LOAD_SYMBOL(rpm, rpmdbGetIteratorCount, 0)
    FF_LIBRARY_LOAD_SYMBOL(rpm, rpmdbFreeIterator, 0)
    FF_LIBRARY_LOAD_SYMBOL(rpm, rpmtsFree, 0)
    FF_LIBRARY_LOAD_SYMBOL(rpm, rpmlogSetMask, 0)

    // Don't print any error messages
    ffrpmlogSetMask(RPMLOG_MASK(RPMLOG_EMERG));

    if(ffrpmReadConfigFiles(NULL, NULL) != 0)
        return 0;

    rpmts ts = ffrpmtsCreate();
    if(ts == NULL)
        return 0;

    rpmdbMatchIterator mi = ffrpmtsInitIterator(ts, RPMDBI_LABEL, NULL, 0);
    if(mi == NULL)
    {
        ffrpmtsFree(ts);
        return 0;
    }

    int count = ffrpmdbGetIteratorCount(mi);

    ffrpmdbFreeIterator(mi);
    ffrpmtsFree(ts);

    return count > 0 ? (uint32_t) count : 0;
}

#endif //FF_HAVE_RPM

static uint32_t getAMPackages(FFstrbuf* baseDir)
{
    uint32_t baseLength = baseDir->length;
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(baseDir->chars);
    if (!dirp) return 0;

    uint32_t result = 0;
    struct dirent *entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (entry->d_name[0] == '.') continue;
        if (entry->d_type == DT_DIR)
        {
            ffStrbufAppendF(baseDir, "/%s/remove", entry->d_name);
            if (ffPathExists(baseDir->chars, FF_PATHTYPE_FILE))
                ++result;
            ffStrbufSubstrBefore(baseDir, baseLength);
        }
    }
    return result;
}

static uint32_t getAMSystem(FFstrbuf* baseDir)
{
    // #771
    uint32_t baseDirLength = baseDir->length;

    ffStrbufAppendS(baseDir, "/opt");
    uint32_t optDirLength = baseDir->length;

    uint32_t result = 0;

    ffStrbufAppendS(baseDir, "/am/APP-MANAGER");
    if (ffPathExists(baseDir->chars, FF_PATHTYPE_FILE))
    {
        ++result; // `am` itself is counted as a package too
        ffStrbufSubstrBefore(baseDir, optDirLength);
        result = getAMPackages(baseDir);
    }

    ffStrbufSubstrBefore(baseDir, baseDirLength);
    return result;
}

static uint32_t getAMUser(void)
{
    if (instance.state.platform.configDirs.length == 0) return 0;

    // check if $XDG_CONFIG_HOME/appman/appman-config exists
    FFstrbuf* baseDir = FF_LIST_GET(FFstrbuf, instance.state.platform.configDirs, 0);
    uint32_t baseLen = baseDir->length;
    ffStrbufAppendS(baseDir, "appman/appman-config");
    FF_STRBUF_AUTO_DESTROY packagesPath = ffStrbufCreate();
    if (ffReadFileBuffer(baseDir->chars, &packagesPath))
        ffStrbufTrimRightSpace(&packagesPath);
    ffStrbufSubstrBefore(baseDir, baseLen);

    return packagesPath.length > 0 ? getAMPackages(&packagesPath) : 0;
}

static int compareHash(const void* a, const void* b)
{
    return memcmp(a, b, 32);
}

static uint32_t getGuixPackagesImpl(char* filename)
{
    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    if (!ffAppendFileBuffer(filename, &content))
        return 0;

    // Count number of unique /gnu/store/ paths in PROFILE/manifest based on their hash value.
    // Contains packages explicitly installed and their propagated inputs.
    char* pend = content.chars;

    for (const char* pattern = content.chars; (pattern = strstr(pattern, "/gnu/store/")); pattern += 32)
    {
        pattern += strlen("/gnu/store/");
        memmove(pend, pattern, 32);
        pend += 32;
    }

    if (pend == content.chars)
        return 0;

    qsort(content.chars, (size_t) (pend - content.chars) / 32, 32, compareHash);

    uint32_t count = 1;
    for (const char* p = content.chars + 32; p < pend; p += 32)
        count += compareHash(p - 32, p) != 0;

    return count;
}

static uint32_t getGuixPackages(FFstrbuf* baseDir, const char* dirname)
{
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, dirname);
    ffStrbufAppendS(baseDir, "/manifest");
    uint32_t num_elements = getGuixPackagesImpl(baseDir->chars);
    ffStrbufSubstrBefore(baseDir, baseDirLength);
    return num_elements;
}

static inline uint32_t getFlatpakRuntimePackagesArch(FFstrbuf* baseDir)
{
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(baseDir->chars);
    if (dirp == NULL)
        return 0;

    uint32_t num_elements = 0;

    struct dirent *entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if(entry->d_type == DT_DIR && entry->d_name[0] != '.')
        {
            num_elements += getNumElements(baseDir, entry->d_name, true);
        }
    }

    return num_elements;
}

static inline uint32_t getFlatpakRuntimePackages(FFstrbuf* baseDir)
{
    ffStrbufAppendS(baseDir, "runtime/");
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(baseDir->chars);
    if (dirp == NULL)
        return 0;

    uint32_t runtimeDirLength = baseDir->length;
    uint32_t num_elements = 0;

    struct dirent *entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if(entry->d_type == DT_DIR && entry->d_name[0] != '.')
        {
            // `flatpak list` ignores `.Locale` and `.Debug` packages, and maybe others
            const char* dot = strrchr(entry->d_name, '.');
            if (__builtin_expect(!dot, false)) continue;
            dot++;

            if (ffStrEquals(dot, "Locale") || ffStrEquals(dot, "Debug"))
                continue;

            ffStrbufAppendS(baseDir, entry->d_name);
            ffStrbufAppendC(baseDir, '/');
            num_elements += getFlatpakRuntimePackagesArch(baseDir);
            ffStrbufSubstrBefore(baseDir, runtimeDirLength);
        }
    }

    return num_elements;
}

static inline uint32_t getFlatpakAppPackages(FFstrbuf* baseDir)
{
    ffStrbufAppendS(baseDir, "app/");
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(baseDir->chars);
    if (dirp == NULL)
        return 0;

    uint32_t appDirLength = baseDir->length;
    uint32_t num_elements = 0;

    struct dirent *entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if(entry->d_type == DT_DIR && entry->d_name[0] != '.')
        {
            ffStrbufAppendS(baseDir, entry->d_name);
            ffStrbufAppendS(baseDir, "/current");
            if (ffPathExists(baseDir->chars, FF_PATHTYPE_ANY)) // Exclude deleted apps, #1856
                ++num_elements;
            ffStrbufSubstrBefore(baseDir, appDirLength);
        }
    }
    return num_elements;
}

static uint32_t getFlatpakPackages(FFstrbuf* baseDir, const char* dirname)
{
    uint32_t num_elements = 0;
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, dirname);
    ffStrbufAppendS(baseDir, "/flatpak/");
    uint32_t flatpakDirLength = baseDir->length;

    num_elements += getFlatpakAppPackages(baseDir);
    ffStrbufSubstrBefore(baseDir, flatpakDirLength);

    num_elements += getFlatpakRuntimePackages(baseDir);

    ffStrbufSubstrBefore(baseDir, baseDirLength);

    return num_elements;
}

static void getPackageCounts(FFstrbuf* baseDir, FFPackagesResult* packageCounts, FFPackagesOptions* options)
{
    if (!(options->disabled & FF_PACKAGES_FLAG_APK_BIT)) packageCounts->apk += getNumStrings(baseDir, "/lib/apk/db/installed", "C:Q", "apk");
    if (!(options->disabled & FF_PACKAGES_FLAG_DPKG_BIT)) packageCounts->dpkg += getNumStrings(baseDir, "/var/lib/dpkg/status", "Status: install ok installed", "dpkg");
    if (!(options->disabled & FF_PACKAGES_FLAG_LPKG_BIT)) packageCounts->lpkg += getNumStrings(baseDir, "/opt/Loc-OS-LPKG/installed-lpkg/Listinstalled-lpkg.list", "\n", "lpkg");
    if (!(options->disabled & FF_PACKAGES_FLAG_EMERGE_BIT)) packageCounts->emerge += countFilesRecursive(baseDir, "/var/db/pkg", "SIZE");
    if (!(options->disabled & FF_PACKAGES_FLAG_EOPKG_BIT)) packageCounts->eopkg += getNumElements(baseDir, "/var/lib/eopkg/package", true);
    if (!(options->disabled & FF_PACKAGES_FLAG_FLATPAK_BIT)) packageCounts->flatpakSystem += getFlatpakPackages(baseDir, "/var/lib");
    if (!(options->disabled & FF_PACKAGES_FLAG_NIX_BIT))
    {
        packageCounts->nixDefault += ffPackagesGetNix(baseDir, "/nix/var/nix/profiles/default");
        packageCounts->nixSystem += ffPackagesGetNix(baseDir, "/run/current-system");
    }
    if (!(options->disabled & FF_PACKAGES_FLAG_PACMAN_BIT)) packageCounts->pacman += getNumElements(baseDir, "/var/lib/pacman/local", true);
    if (!(options->disabled & FF_PACKAGES_FLAG_LPKGBUILD_BIT)) packageCounts->lpkgbuild += getNumElements(baseDir, "/opt/Loc-OS-LPKG/lpkgbuild/remove", false);
    if (!(options->disabled & FF_PACKAGES_FLAG_PKGTOOL_BIT)) packageCounts->pkgtool += getNumElements(baseDir, "/var/log/packages", false);
    if (!(options->disabled & FF_PACKAGES_FLAG_RPM_BIT))
    {
        // `Sigmd5` is the only table that doesn't contain the virtual `gpg-pubkey` package
        packageCounts->rpm += getSQLite3Int(baseDir, "/var/lib/rpm/rpmdb.sqlite", "SELECT count(*) FROM Sigmd5", "rpm");
    }
    if (!(options->disabled & FF_PACKAGES_FLAG_SNAP_BIT)) packageCounts->snap += getSnap(baseDir);
    if (!(options->disabled & FF_PACKAGES_FLAG_XBPS_BIT)) packageCounts->xbps += getXBPS(baseDir, "/var/db/xbps");
    if (!(options->disabled & FF_PACKAGES_FLAG_BREW_BIT))
    {
        packageCounts->brewCask += getNumElements(baseDir, "/home/linuxbrew/.linuxbrew/Caskroom", true);
        packageCounts->brew += getNumElements(baseDir, "/home/linuxbrew/.linuxbrew/Cellar", true);
    }
    if (!(options->disabled & FF_PACKAGES_FLAG_PALUDIS_BIT)) packageCounts->paludis += countFilesRecursive(baseDir, "/var/db/paludis/repositories", "environment.bz2");
    if (!(options->disabled & FF_PACKAGES_FLAG_OPKG_BIT)) packageCounts->opkg += getNumStrings(baseDir, "/usr/lib/opkg/status", "Package:", "opkg"); // openwrt
    if (!(options->disabled & FF_PACKAGES_FLAG_AM_BIT)) packageCounts->amSystem = getAMSystem(baseDir);
    if (!(options->disabled & FF_PACKAGES_FLAG_SORCERY_BIT)) packageCounts->sorcery += getNumStrings(baseDir, "/var/state/sorcery/packages", ":installed:", "sorcery");
    if (!(options->disabled & FF_PACKAGES_FLAG_GUIX_BIT))
    {
      packageCounts->guixSystem += getGuixPackages(baseDir, "/run/current-system/profile");
    }
    if (!(options->disabled & FF_PACKAGES_FLAG_LINGLONG_BIT)) packageCounts->linglong += getNumElements(baseDir, "/var/lib/linglong/layers", true);
    if (!(options->disabled & FF_PACKAGES_FLAG_PACSTALL_BIT)) packageCounts->pacstall += getNumElements(baseDir, "/var/lib/pacstall/metadata", false);
    if (!(options->disabled & FF_PACKAGES_FLAG_PISI_BIT)) packageCounts->pisi += getNumElements(baseDir, "/var/lib/pisi/package", true);
    if (!(options->disabled & FF_PACKAGES_FLAG_PKGSRC_BIT)) packageCounts->pkgsrc += getNumElements(baseDir, "/usr/pkg/pkgdb", DT_DIR);
}

static void getPackageCountsRegular(FFstrbuf* baseDir, FFPackagesResult* packageCounts, FFPackagesOptions* options)
{
    getPackageCounts(baseDir, packageCounts, options);

    if (!(options->disabled & FF_PACKAGES_FLAG_PACMAN_BIT))
    {
        uint32_t baseDirLength = baseDir->length;
        ffStrbufAppendS(baseDir, FASTFETCH_TARGET_DIR_ETC "/pacman-mirrors.conf");
        if(ffParsePropFile(baseDir->chars, "Branch =", &packageCounts->pacmanBranch) && packageCounts->pacmanBranch.length == 0)
            ffStrbufAppendS(&packageCounts->pacmanBranch, "stable");
        ffStrbufSubstrBefore(baseDir, baseDirLength);
    }
}

static void getPackageCountsBedrock(FFstrbuf* baseDir, FFPackagesResult* packageCounts, FFPackagesOptions* options)
{
    uint32_t baseDirLength = baseDir->length;

    ffStrbufAppendS(baseDir, "/bedrock/strata");

    FF_AUTO_CLOSE_DIR DIR* dir = opendir(baseDir->chars);
    if(dir == NULL)
    {
        ffStrbufSubstrBefore(baseDir, baseDirLength);
        return;
    }

    ffStrbufAppendC(baseDir, '/');
    uint32_t baseDirLength2 = baseDir->length;

    struct dirent* entry;
    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_type != DT_DIR)
            continue;
        if(entry->d_name[0] == '.')
            continue;

        ffStrbufAppendS(baseDir, entry->d_name);
        getPackageCounts(baseDir, packageCounts, options);
        ffStrbufSubstrBefore(baseDir, baseDirLength2);
    }

    ffStrbufSubstrBefore(baseDir, baseDirLength);
}

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options)
{
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateA(512);
    ffStrbufAppendS(&baseDir, FASTFETCH_TARGET_DIR_ROOT);

    if(ffStrbufIgnCaseEqualS(&ffDetectOS()->id, "bedrock"))
        getPackageCountsBedrock(&baseDir, result, options);
    else
        getPackageCountsRegular(&baseDir, result, options);

    // If SQL failed, we can still try with librpm.
    // This is needed on openSUSE, which seems to use a proprietary database file
    // This method doesn't work on bedrock, so we do it here.
    #ifdef FF_HAVE_RPM
        if(!(options->disabled & FF_PACKAGES_FLAG_RPM_BIT) && result->rpm == 0)
            result->rpm = getRpmFromLibrpm();
    #endif

    ffStrbufSet(&baseDir, &instance.state.platform.homeDir);
    if (!(options->disabled & FF_PACKAGES_FLAG_NIX_BIT))
    {
        // Count packages from $HOME/.nix-profile
        result->nixUser += ffPackagesGetNix(&baseDir, ".nix-profile");

        // Check in $XDG_STATE_HOME/nix/profile
        FF_STRBUF_AUTO_DESTROY stateHome = ffStrbufCreate();
        const char* stateHomeEnv = getenv("XDG_STATE_HOME");
        if (ffStrSet(stateHomeEnv))
        {
            ffStrbufSetS(&stateHome, stateHomeEnv);
            ffStrbufEnsureEndsWithC(&stateHome, '/');
        }
        else
        {
            ffStrbufSet(&stateHome, &instance.state.platform.homeDir);
            ffStrbufAppendS(&stateHome, ".local/state/");
        }
        result->nixUser += ffPackagesGetNix(&stateHome, "nix/profile");

        // Check in /etc/profiles/per-user/$USER
        FF_STRBUF_AUTO_DESTROY userPkgsDir = ffStrbufCreateStatic("/etc/profiles/per-user/");
        result->nixUser += ffPackagesGetNix(&userPkgsDir, instance.state.platform.userName.chars);
    }

    if (!(options->disabled & FF_PACKAGES_FLAG_GUIX_BIT))
    {
       result->guixUser += getGuixPackages(&baseDir, ".guix-profile");
       result->guixHome += getGuixPackages(&baseDir, ".guix-home/profile");
    }

    if (!(options->disabled & FF_PACKAGES_FLAG_FLATPAK_BIT))
        result->flatpakUser = getFlatpakPackages(&baseDir, "/.local/share");

    if (!(options->disabled & FF_PACKAGES_FLAG_AM_BIT))
        result->amUser = getAMUser();

    if (!(options->disabled & FF_PACKAGES_FLAG_SOAR_BIT))
        result->soar += getSQLite3Int(&baseDir, ".local/share/soar/db/soar.db", "SELECT COUNT(DISTINCT pkg_id || pkg_name) FROM packages WHERE is_installed = true", "soar");
}
