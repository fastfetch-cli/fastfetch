#include "fastfetch.h"

#include <string.h>
#include <dirent.h>

#define FF_PACKAGES_MODULE_NAME "Packages"
#define FF_PACKAGES_NUM_FORMAT_ARGS 9

#ifdef FF_HAVE_RPM
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlog.h>

static uint32_t getRpmFromLibrpm(const FFinstance* instance)
{
    FF_LIBRARY_LOAD(rpm, instance->config.librpm, 0, "librpm.so", 12)
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
    {
        dlclose(rpm);
        return 0;
    }

    rpmts ts = ffrpmtsCreate();
    if(ts == NULL)
    {
        dlclose(rpm);
        return 0;
    }

    rpmdbMatchIterator mi = ffrpmtsInitIterator(ts, RPMDBI_LABEL, NULL, 0);
    if(mi == NULL)
    {
        ffrpmtsFree(ts);
        dlclose(rpm);
        return 0;
    }

    int count = ffrpmdbGetIteratorCount(mi);

    ffrpmdbFreeIterator(mi);
    ffrpmtsFree(ts);
    dlclose(rpm);

    return count > 0 ? (uint32_t) count : 0;
}

#endif

static uint32_t getNumElements(const char* dirname, unsigned char type)
{
    DIR* dirp = opendir(dirname);
    if(dirp == NULL)
        return 0;

    uint32_t num_elements = 0;

    struct dirent *entry;
    while((entry = readdir(dirp)) != NULL) {
        if(entry->d_type == type)
            ++num_elements;
    }

    if(type == DT_DIR)
        num_elements -= 2; // accounting for . and ..

    closedir(dirp);

    return num_elements;
}

static uint32_t getNumStrings(const char* filename, const char* needle)
{
    FILE* file = fopen(filename, "r");
    if(file == NULL)
        return 0;

    uint32_t count = 0;

    char* line = NULL;
    size_t len = 0;

    while(getline(&line, &len, file) != EOF)
    {
        if(strstr(line, needle) != NULL)
            ++count;
    }

    if(line != NULL)
        free(line);

    fclose(file);

    return count;
}

static uint32_t countFilesRecursive(FFstrbuf* baseDirPath, const char* filename)
{
    uint32_t baseDirPathLength = baseDirPath->length;

    ffStrbufAppendC(baseDirPath, '/');
    ffStrbufAppendS(baseDirPath, filename);
    bool exists = ffFileExists(baseDirPath->chars, S_IFREG);
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
        sum += countFilesRecursive(baseDirPath, filename);
        ffStrbufSubstrBefore(baseDirPath, baseDirPathLength);
    }

    closedir(dirp);
    return sum;
}

static uint32_t countFilesIn(const char* dirname, const char* filename)
{
    FFstrbuf baseDirPath;
    ffStrbufInitA(&baseDirPath, 128);
    ffStrbufAppendS(&baseDirPath, dirname);
    uint32_t result = countFilesRecursive(&baseDirPath, filename);
    ffStrbufDestroy(&baseDirPath);
    return result;
}

static uint32_t getNixPackages(const FFstrbuf* path)
{
    FFstrbuf output;
    ffStrbufInitA(&output, 128);

    ffProcessAppendStdOut(&output, (char* const[]) {
        "nix-store",
        "-qR",
        path->chars,
        NULL
    });

    //Each package is a new line in the output. If at least one line is found, add 1 for the last line.
    uint32_t result = ffStrbufCountC(&output, '\n');
    if(result > 0)
        result++;

    ffStrbufDestroy(&output);
    return result;
}

static uint32_t getNixPackagesDefault()
{
    FFstrbuf path;
    ffStrbufInitA(&path, 64);
    ffStrbufAppendS(&path, FASTFETCH_TARGET_DIR_ROOT"/nix/var/nix/profiles/default");
    uint32_t result = getNixPackages(&path);
    ffStrbufDestroy(&path);
    return result;
}

static uint32_t getNixPackagesUser(const FFinstance* instance)
{
    FFstrbuf path;
    ffStrbufInitA(&path, 64);
    ffStrbufAppendS(&path, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&path, "/.nix-profile");
    uint32_t result = getNixPackages(&path);
    ffStrbufDestroy(&path);
    return result;
}

static uint32_t getRpmPackageCount(const FFinstance* instance)
{
    // We do our own sql querys instead of using (lib)rpm, because:
    // - simply faster
    // - we don't need to regulary increase librpm version, which increases rapidly without providing a major version symlink
    uint32_t rpm = (uint32_t) ffSettingsGetSQLite3Int(instance, FASTFETCH_TARGET_DIR_ROOT"/var/lib/rpm/rpmdb.sqlite", "SELECT count(blob) FROM Packages");
    if(rpm != 0)
        return rpm;

    rpm = (uint32_t) ffSettingsGetSQLite3Int(instance, FASTFETCH_TARGET_DIR_ROOT"/var/cache/dnf/packages.db", "SELECT count(pkg) FROM installed");
    if(rpm != 0)
        return rpm;

    // If SQL failed, we can still try with rpm.
    // This is needed on openSUSE, which seems to use a proprietary database file
    #ifdef FF_HAVE_RPM
        return getRpmFromLibrpm(instance);
    #endif

    return 0;
}

void ffPrintPackages(FFinstance* instance)
{
    uint32_t pacman = getNumElements(FASTFETCH_TARGET_DIR_ROOT"/var/lib/pacman/local", DT_DIR);
    uint32_t dpkg = getNumStrings(FASTFETCH_TARGET_DIR_ROOT"/var/lib/dpkg/status", "Status: ");
    uint32_t rpm = getRpmPackageCount(instance);
    uint32_t emerge = countFilesIn(FASTFETCH_TARGET_DIR_ROOT"/var/db/pkg", "SIZE");
    uint32_t xbps = getNumElements(FASTFETCH_TARGET_DIR_ROOT"/var/db/xbps", DT_REG);

    uint32_t nixUser = 0;
    uint32_t nixDefault = 0;

    //Nix detection is kinda slow, so we only do it if the nix dir exists
    if(ffFileExists(FASTFETCH_TARGET_DIR_ROOT"/nix", S_IFDIR))
    {
        nixUser = getNixPackagesUser(instance);
        nixDefault = getNixPackagesDefault();
    }

    uint32_t flatpak = getNumElements(FASTFETCH_TARGET_DIR_ROOT"/var/lib/flatpak/app", DT_DIR);
    uint32_t snap = getNumElements(FASTFETCH_TARGET_DIR_ROOT"/snap", DT_DIR);

    //Accounting for the /snap/bin folder
    if(snap > 0)
        --snap;

    uint32_t all = pacman + dpkg + rpm + emerge + xbps + nixUser + nixDefault + flatpak + snap;

    if(all == 0)
    {
        ffPrintError(instance, FF_PACKAGES_MODULE_NAME, 0, &instance->config.packagesKey, &instance->config.packagesFormat, FF_PACKAGES_NUM_FORMAT_ARGS, "No packages from known package managers found");
        return;
    }

    FFstrbuf manjaroBranch;
    ffStrbufInit(&manjaroBranch);
    if(ffParsePropFile(FASTFETCH_TARGET_DIR_ROOT"/etc/pacman-mirrors.conf", "Branch =", &manjaroBranch) && manjaroBranch.length == 0)
        ffStrbufSetS(&manjaroBranch, "stable");

    if(instance->config.packagesFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PACKAGES_MODULE_NAME, 0, &instance->config.packagesKey);

        #define FF_PRINT_PACKAGE(name) \
        if(name > 0) \
        { \
            printf("%u ("#name")", name); \
            if((all = all - name) > 0) \
                printf(", "); \
        };

        if(pacman > 0)
        {
            printf("%u (pacman)", pacman);
            if(manjaroBranch.length > 0)
                printf("[%s]", manjaroBranch.chars);
            if((all = all - pacman) > 0)
                printf(", ");
        };

        FF_PRINT_PACKAGE(dpkg)
        FF_PRINT_PACKAGE(rpm)
        FF_PRINT_PACKAGE(emerge)

        if(nixUser > 0)
        {
            printf("%u (nix-user)", nixUser);
            if((all = all - nixUser) > 0)
                printf(", ");
        }

        if(nixDefault > 0)
        {
            printf("%u (nix-default)", nixDefault);
            if((all = all - nixDefault) > 0)
                printf(", ");
        }

        FF_PRINT_PACKAGE(flatpak)
        FF_PRINT_PACKAGE(snap)

        //Fix linter warning of unused value of all
        (void) all;

        #undef FF_PRINT_PACKAGE

        putchar('\n');
    }
    else
    {
        ffPrintFormatString(instance, FF_PACKAGES_MODULE_NAME, 0, &instance->config.packagesKey, &instance->config.packagesFormat, NULL, FF_PACKAGES_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT, &all},
            {FF_FORMAT_ARG_TYPE_UINT, &pacman},
            {FF_FORMAT_ARG_TYPE_STRBUF, &manjaroBranch},
            {FF_FORMAT_ARG_TYPE_UINT, &dpkg},
            {FF_FORMAT_ARG_TYPE_UINT, &rpm},
            {FF_FORMAT_ARG_TYPE_UINT, &emerge},
            {FF_FORMAT_ARG_TYPE_UINT, &xbps},
            {FF_FORMAT_ARG_TYPE_UINT, &flatpak},
            {FF_FORMAT_ARG_TYPE_UINT, &snap}
        });
    }

    ffStrbufDestroy(&manjaroBranch);
}
