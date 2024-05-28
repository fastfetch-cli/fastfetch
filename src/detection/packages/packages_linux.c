#include "packages.h"
#include "common/io/io.h"
#include "common/parsing.h"
#include "common/processing.h"
#include "common/properties.h"
#include "common/settings.h"
#include "detection/os/os.h"
#include "util/stringUtils.h"

#include <dirent.h>

static uint32_t getNumElementsImpl(const char* dirname, unsigned char type)
{
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(dirname);
    if(dirp == NULL)
        return 0;

    uint32_t num_elements = 0;

    struct dirent *entry;
    while((entry = readdir(dirp)) != NULL) {
        if(entry->d_type == type)
            ++num_elements;
    }

    if(type == DT_DIR && num_elements >= 2)
        num_elements -= 2; // accounting for . and ..

    return num_elements;
}

static uint32_t getNumElements(FFstrbuf* baseDir, const char* dirname, unsigned char type)
{
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, dirname);
    uint32_t num_elements = getNumElementsImpl(baseDir->chars, type);
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

static uint32_t getNumStrings(FFstrbuf* baseDir, const char* filename, const char* needle)
{
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, filename);
    uint32_t num_elements = getNumStringsImpl(baseDir->chars, needle);
    ffStrbufSubstrBefore(baseDir, baseDirLength);
    return num_elements;
}

static uint32_t getSQLite3Int(FFstrbuf* baseDir, const char* dbPath, const char* query)
{
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, dbPath);
    uint32_t num_elements = (uint32_t) ffSettingsGetSQLite3Int(baseDir->chars, query);
    ffStrbufSubstrBefore(baseDir, baseDirLength);
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

static bool isValidNixPkg(FFstrbuf* pkg)
{
    if (!ffPathExists(pkg->chars, FF_PATHTYPE_DIRECTORY))
        return false;

    ffStrbufSubstrAfterLastC(pkg, '/');
    if (
        ffStrbufStartsWithS(pkg, "nixos-system-nixos-") ||
        ffStrbufEndsWithS(pkg, "-doc") ||
        ffStrbufEndsWithS(pkg, "-man") ||
        ffStrbufEndsWithS(pkg, "-info") ||
        ffStrbufEndsWithS(pkg, "-dev") ||
        ffStrbufEndsWithS(pkg, "-bin")
    ) return false;

    enum { START, DIGIT, DOT, MATCH } state = START;

    for (uint32_t i = 0; i < pkg->length; i++)
    {
        char c = pkg->chars[i];
        switch (state)
        {
            case START:
                if (ffCharIsDigit(c))
                    state = DIGIT;
                break;
            case DIGIT:
                if (ffCharIsDigit(c))
                    continue;
                if (c == '.')
                    state = DOT;
                else
                    state = START;
                break;
            case DOT:
                if (ffCharIsDigit(c))
                    state = MATCH;
                else
                    state = START;
                break;
            case MATCH:
                break;
        }
    }

    return state == MATCH;
}

static bool checkNixCache(FFstrbuf* cacheDir, FFstrbuf* hash, uint32_t* count)
{
    if (!ffPathExists(cacheDir->chars, FF_PATHTYPE_FILE))
        return false;

    FF_STRBUF_AUTO_DESTROY cacheContent = ffStrbufCreate();
    if (!ffReadFileBuffer(cacheDir->chars, &cacheContent))
        return false;

    // Format: <hash>\n<count>
    uint32_t split = ffStrbufFirstIndexC(&cacheContent, '\n');
    if (split == cacheContent.length)
        return false;

    ffStrbufSetNS(hash, split, cacheContent.chars);
    *count = (uint32_t)atoi(cacheContent.chars + split + 1);

    return true;
}

static bool writeNixCache(FFstrbuf* cacheDir, FFstrbuf* hash, uint32_t count)
{
    FF_STRBUF_AUTO_DESTROY cacheContent = ffStrbufCreateCopy(hash);
    ffStrbufAppendF(&cacheContent, "\n%u", count);
    return ffWriteFileBuffer(cacheDir->chars, &cacheContent);
}

static uint32_t getNixPackagesImpl(char* path)
{
    //Nix detection is kinda slow, so we only do it if the dir exists
    if(!ffPathExists(path, FF_PATHTYPE_DIRECTORY))
        return 0;

    FF_STRBUF_AUTO_DESTROY cacheDir = ffStrbufCreateCopy(&instance.state.platform.cacheDir);
    ffStrbufEnsureEndsWithC(&cacheDir, '/');
    ffStrbufAppendS(&cacheDir, "fastfetch/packages/nix");
    ffStrbufAppendS(&cacheDir, path);

    //Check the hash first to determine if we need to recompute the count
    FF_STRBUF_AUTO_DESTROY hash = ffStrbufCreateA(64);
    FF_STRBUF_AUTO_DESTROY cacheHash = ffStrbufCreateA(64);
    uint32_t count = 0;

    ffProcessAppendStdOut(&hash, (char* const[]) {
        "nix-store",
        "--query",
        "--hash",
        path,
        NULL
    });

    if (checkNixCache(&cacheDir, &cacheHash, &count) && ffStrbufEqual(&hash, &cacheHash))
        return count;

    //Cache is invalid, recompute the count
    count = 0;

    //Implementation based on bash script from here:
    //https://github.com/fastfetch-cli/fastfetch/issues/195#issuecomment-1191748222

    FF_STRBUF_AUTO_DESTROY output = ffStrbufCreateA(1024);

    ffProcessAppendStdOut(&output, (char* const[]) {
        "nix-store",
        "--query",
        "--requisites",
        path,
        NULL
    });

    uint32_t lineLength = 0;
    for (uint32_t i = 0; i < output.length; i++)
    {
        if (output.chars[i] != '\n')
        {
            lineLength++;
            continue;
        }

        output.chars[i] = '\0';
        FFstrbuf line = {
            .allocated = 0,
            .length = lineLength,
            .chars = output.chars + i - lineLength
        };
        if (isValidNixPkg(&line))
            count++;
        lineLength = 0;
    }

    writeNixCache(&cacheDir, &hash, count);
    return count;
}

static uint32_t getNixPackages(FFstrbuf* baseDir, const char* dirname)
{
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, dirname);
    uint32_t num_elements = getNixPackagesImpl(baseDir->chars);
    ffStrbufSubstrBefore(baseDir, baseDirLength);
    return num_elements;
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
    uint32_t result = getNumElements(baseDir, "/snap", DT_DIR);

    if (result == 0)
        result = getNumElements(baseDir, "/var/lib/snapd/snap", DT_DIR);

    //Accounting for the /snap/bin folder
    return result > 0 ? result - 1 : 0;
}

static uint32_t getFlatpak(FFstrbuf* baseDir, const char* dirname)
{
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, dirname);

    uint32_t result =
        getNumElements(baseDir, "/app", DT_DIR) +
        getNumElements(baseDir, "/runtime", DT_DIR);

    ffStrbufSubstrBefore(baseDir, baseDirLength);
    return result;
}

#ifdef FF_HAVE_RPM
#include "common/library.h"
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlog.h>

static uint32_t getRpmFromLibrpm(void)
{
    FF_LIBRARY_LOAD(rpm, &instance.config.library.librpm, 0, "librpm" FF_LIBRARY_EXTENSION, 12)
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

static uint32_t getAM(FFstrbuf* baseDir)
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
        FF_AUTO_CLOSE_DIR DIR* dirp = opendir(baseDir->chars);
        if(dirp)
        {
            struct dirent *entry;
            while ((entry = readdir(dirp)) != NULL)
            {
                if (entry->d_name[0] == '.') continue;
                if (entry->d_type == DT_DIR)
                {
                    ffStrbufAppendF(baseDir, "/%s/AM-updater", entry->d_name);
                    if (ffPathExists(baseDir->chars, FF_PATHTYPE_FILE))
                        ++result;
                    ffStrbufSubstrBefore(baseDir, optDirLength);
                }
            }
        }
    }

    ffStrbufSubstrBefore(baseDir, baseDirLength);
    return result;
}


static uint32_t getGuixPackagesImpl(char* path)
{
    FF_STRBUF_AUTO_DESTROY output = ffStrbufCreateA(1024);

    ffProcessAppendStdOut(&output, (char* const[]) {
        "guix",
        "package",
        "-p",
        path,
        "-I",
        NULL
    });


    //Each package is a new line in the output.
    // If at least one line is found, add 1 for the last line.
    uint32_t count = ffStrbufCountC(&output, '\n');
    if(count > 0)
      count++;

    return count;
}

static uint32_t getGuixPackages(FFstrbuf* baseDir, const char* dirname)
{
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, dirname);
    uint32_t num_elements = getGuixPackagesImpl(baseDir->chars);
    ffStrbufSubstrBefore(baseDir, baseDirLength);
    return num_elements;
}

static void getPackageCounts(FFstrbuf* baseDir, FFPackagesResult* packageCounts, FFPackagesOptions* options)
{
    if (!(options->disabled & FF_PACKAGES_FLAG_APK_BIT)) packageCounts->apk += getNumStrings(baseDir, "/lib/apk/db/installed", "C:Q");
    if (!(options->disabled & FF_PACKAGES_FLAG_DPKG_BIT)) packageCounts->dpkg += getNumStrings(baseDir, "/var/lib/dpkg/status", "Status: install ok installed");
    if (!(options->disabled & FF_PACKAGES_FLAG_LPKG_BIT)) packageCounts->lpkg += getNumStrings(baseDir, "/opt/Loc-OS-LPKG/installed-lpkg/Listinstalled-lpkg.list", "\n");
    if (!(options->disabled & FF_PACKAGES_FLAG_EMERGE_BIT)) packageCounts->emerge += countFilesRecursive(baseDir, "/var/db/pkg", "SIZE");
    if (!(options->disabled & FF_PACKAGES_FLAG_EOPKG_BIT)) packageCounts->eopkg += getNumElements(baseDir, "/var/lib/eopkg/package", DT_DIR);
    if (!(options->disabled & FF_PACKAGES_FLAG_FLATPAK_BIT)) packageCounts->flatpakSystem += getFlatpak(baseDir, "/var/lib/flatpak");
    if (!(options->disabled & FF_PACKAGES_FLAG_NIX_BIT))
    {
        packageCounts->nixDefault += getNixPackages(baseDir, "/nix/var/nix/profiles/default");
        packageCounts->nixSystem += getNixPackages(baseDir, "/run/current-system");
    }
    if (!(options->disabled & FF_PACKAGES_FLAG_PACMAN_BIT)) packageCounts->pacman += getNumElements(baseDir, "/var/lib/pacman/local", DT_DIR);
    if (!(options->disabled & FF_PACKAGES_FLAG_LPKGBUILD_BIT)) packageCounts->lpkgbuild += getNumElements(baseDir, "/opt/Loc-OS-LPKG/lpkgbuild/remove", DT_REG);
    if (!(options->disabled & FF_PACKAGES_FLAG_PKGTOOL_BIT)) packageCounts->pkgtool += getNumElements(baseDir, "/var/log/packages", DT_REG);
    if (!(options->disabled & FF_PACKAGES_FLAG_RPM_BIT)) packageCounts->rpm += getSQLite3Int(baseDir, "/var/lib/rpm/rpmdb.sqlite", "SELECT count(*) FROM Packages");
    if (!(options->disabled & FF_PACKAGES_FLAG_SNAP_BIT)) packageCounts->snap += getSnap(baseDir);
    if (!(options->disabled & FF_PACKAGES_FLAG_XBPS_BIT)) packageCounts->xbps += getXBPS(baseDir, "/var/db/xbps");
    if (!(options->disabled & FF_PACKAGES_FLAG_BREW_BIT))
    {
        packageCounts->brewCask += getNumElements(baseDir, "/home/linuxbrew/.linuxbrew/Caskroom", DT_DIR);
        packageCounts->brew += getNumElements(baseDir, "/home/linuxbrew/.linuxbrew/Cellar", DT_DIR);
    }
    if (!(options->disabled & FF_PACKAGES_FLAG_PALUDIS_BIT)) packageCounts->paludis += countFilesRecursive(baseDir, "/var/db/paludis/repositories", "environment.bz2");
    if (!(options->disabled & FF_PACKAGES_FLAG_OPKG_BIT)) packageCounts->opkg += getNumStrings(baseDir, "/usr/lib/opkg/status", "Package:"); // openwrt
    if (!(options->disabled & FF_PACKAGES_FLAG_AM_BIT)) packageCounts->am = getAM(baseDir);
    if (!(options->disabled & FF_PACKAGES_FLAG_SORCERY_BIT)) packageCounts->sorcery += getNumStrings(baseDir, "/var/state/sorcery/packages", ":installed:");
    if (!(options->disabled & FF_PACKAGES_FLAG_GUIX_BIT))
    {
      packageCounts->guixSystem += getGuixPackages(baseDir, "/run/current-system/profile");
    }
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
        if(ffStrEquals(entry->d_name, ".") || ffStrEquals(entry->d_name, ".."))
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
        // check if ~/.nix-profile exists
        FF_STRBUF_AUTO_DESTROY profilePath = ffStrbufCreateCopy(&baseDir);
        ffStrbufAppendS(&profilePath, ".nix-profile");
        if (ffPathExists(profilePath.chars, FF_PATHTYPE_DIRECTORY))
        {
            result->nixUser += getNixPackages(&baseDir, ".nix-profile");
        }

        // check if $XDG_STATE_HOME/nix/profile exists
        FF_STRBUF_AUTO_DESTROY stateDir = ffStrbufCreate();
        const char* stateHome = getenv("XDG_STATE_HOME");
        if(ffStrSet(stateHome))
        {
            ffStrbufSetS(&stateDir, stateHome);
            ffStrbufEnsureEndsWithC(&stateDir, '/');
        }
        else
        {
            ffStrbufSet(&stateDir, &instance.state.platform.homeDir);
            ffStrbufAppendS(&stateDir, ".local/state/");
        }

        ffStrbufSet(&profilePath, &stateDir);
        ffStrbufAppendS(&profilePath, "nix/profile");
        result->nixUser += getNixPackages(&stateDir, "nix/profile");
    }

    if (!(options->disabled & FF_PACKAGES_FLAG_GUIX_BIT))
    {
       result->guixUser += getGuixPackages(&baseDir, ".guix-profile");
       result->guixHome += getGuixPackages(&baseDir, ".guix-home/profile");
    }

    if (!(options->disabled & FF_PACKAGES_FLAG_FLATPAK_BIT))
        result->flatpakUser = getFlatpak(&baseDir, "/.local/share/flatpak");
}
