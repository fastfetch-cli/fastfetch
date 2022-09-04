#include "fastfetch.h"
#include "common/io.h"
#include "common/properties.h"
#include "common/printing.h"
#include "common/settings.h"
#include "common/processing.h"
#include "detection/os/os.h"

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define FF_PACKAGES_MODULE_NAME "Packages"
#define FF_PACKAGES_NUM_FORMAT_ARGS 13

typedef struct PackageCounts
{
    uint32_t pacman;
    uint32_t dpkg;
    uint32_t rpm;
    uint32_t emerge;
    uint32_t xbps;
    uint32_t nixSystem;
    uint32_t nixDefault;
    uint32_t apk;
    uint32_t flatpak;
    uint32_t snap;

    FFstrbuf pacmanBranch;
} PackageCounts;

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

static uint32_t getNixPackages(char* path)
{
    //Nix detection is kinda slow, so we only do it if the dir exists
    if(!ffFileExists(path, S_IFDIR))
        return 0;

    FFstrbuf output;
    ffStrbufInitA(&output, 128);

    //https://github.com/LinusDierheimer/fastfetch/issues/195#issuecomment-1191748222
    FFstrbuf command;
    ffStrbufInitA(&command, 255);
    ffStrbufAppendS(&command, "for x in $(nix-store --query --requisites ");
    ffStrbufAppendS(&command, path);
    ffStrbufAppendS(&command, "); do if [ -d $x ]; then echo $x ; fi ; done | cut -d- -f2- | egrep '([0-9]{1,}\\.)+[0-9]{1,}' | egrep -v '\\-doc$|\\-man$|\\-info$|\\-dev$|\\-bin$|^nixos-system-nixos-' | uniq");

    ffProcessAppendStdOut(&output, (char* const[]) {
        "sh",
        "-c",
        command.chars,
        NULL
    });

    //Each package is a new line in the output. If at least one line is found, add 1 for the last line.
    uint32_t result = ffStrbufCountC(&output, '\n');
    if(result > 0)
        result++;

    ffStrbufDestroy(&output);
    return result;
}

#ifdef FF_HAVE_RPM
#include "common/library.h"
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

static uint32_t getXBPS(FFstrbuf* baseDir)
{
    DIR* dir = opendir(baseDir->chars);
    if(dir == NULL)
        return 0;

    uint32_t result = 0;

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_type != DT_REG || strncasecmp(entry->d_name, "pkgdb-", 6) != 0)
            continue;

        ffStrbufAppendC(baseDir, '/');
        ffStrbufAppendS(baseDir, entry->d_name);
        result = getNumStrings(baseDir->chars, "<string>installed</string>");
        break;
    }

    closedir(dir);
    return result;
}

static void getPackageCounts(const FFinstance* instance, FFstrbuf* baseDir, PackageCounts* packageCounts)
{
    uint32_t baseDirLength = baseDir->length;

    //pacman
    ffStrbufAppendS(baseDir, "/var/lib/pacman/local");
    packageCounts->pacman += getNumElements(baseDir->chars, DT_DIR);
    ffStrbufSubstrBefore(baseDir, baseDirLength);

    //dpkg
    ffStrbufAppendS(baseDir, "/var/lib/dpkg/status");
    packageCounts->dpkg += getNumStrings(baseDir->chars, "Status: ");
    ffStrbufSubstrBefore(baseDir, baseDirLength);

    //rpm
    ffStrbufAppendS(baseDir, "/var/lib/rpm/rmpdb.sqlite");
    packageCounts->rpm += (uint32_t) ffSettingsGetSQLite3Int(instance, baseDir->chars, "SELECT count(blob) FROM Packages");
    ffStrbufSubstrBefore(baseDir, baseDirLength);

    //emerge
    ffStrbufAppendS(baseDir, "/var/db/pkg");
    packageCounts->emerge += countFilesRecursive(baseDir, "SIZE");
    ffStrbufSubstrBefore(baseDir, baseDirLength);

    //xps
    ffStrbufAppendS(baseDir, "/var/db/xbps");
    packageCounts->xbps += getXBPS(baseDir);
    ffStrbufSubstrBefore(baseDir, baseDirLength);

    //nix system
    ffStrbufAppendS(baseDir, "/run/current-system");
    packageCounts->nixSystem += getNixPackages(baseDir->chars);
    ffStrbufSubstrBefore(baseDir, baseDirLength);

    //nix default
    ffStrbufAppendS(baseDir, "/nix/var/nix/profiles/default");
    packageCounts->nixDefault += getNixPackages(baseDir->chars);
    ffStrbufSubstrBefore(baseDir, baseDirLength);

    //apk
    ffStrbufAppendS(baseDir, "/lib/apk/db/installed");
    packageCounts->apk += getNumStrings(baseDir->chars, "C:Q");
    ffStrbufSubstrBefore(baseDir, baseDirLength);

    //flatpak
    ffStrbufAppendS(baseDir, "/var/lib/flatpak/app");
    packageCounts->flatpak += getNumElements(baseDir->chars, DT_DIR);
    ffStrbufSubstrBefore(baseDir, baseDirLength);

    //snap
    ffStrbufAppendS(baseDir, "/snap");
    uint32_t snap = getNumElements(baseDir->chars, DT_DIR);
    if(snap > 0)
        packageCounts->snap += (snap - 1); //Accounting for the /snap/bin folder
    ffStrbufSubstrBefore(baseDir, baseDirLength);

    //pacman branch
    ffStrbufAppendS(baseDir, "/etc/pacman-mirrors.conf");
    if(ffParsePropFile(baseDir->chars, "Branch =", &packageCounts->pacmanBranch) && packageCounts->pacmanBranch.length == 0)
        ffStrbufAppendS(&packageCounts->pacmanBranch, "stable");
    ffStrbufSubstrBefore(baseDir, baseDirLength);
}

static void getPackageCountsBedrock(const FFinstance* instance, FFstrbuf* baseDir, PackageCounts* packageCounts)
{
    uint32_t baseDirLength = baseDir->length;

    ffStrbufAppendS(baseDir, "/bedrock/strata");

    DIR* dir = opendir(baseDir->chars);
    if(dir == NULL)
    {
        ffStrbufSubstrBefore(baseDir, baseDirLength);
        getPackageCounts(instance, baseDir, packageCounts);
        return;
    }

    ffStrbufAppendC(baseDir, '/');
    baseDirLength = baseDir->length;

    struct dirent* entry;
    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_type != DT_DIR)
            continue;

        ffStrbufAppendS(baseDir, entry->d_name);
        getPackageCounts(instance, baseDir, packageCounts);
        ffStrbufSubstrBefore(baseDir, baseDirLength);
    }

    closedir(dir);
}

void ffPrintPackages(FFinstance* instance)
{
    PackageCounts counts = {0};
    ffStrbufInit(&counts.pacmanBranch);

    FFstrbuf baseDir;
    ffStrbufInitA(&baseDir, 512);
    ffStrbufAppendS(&baseDir, FASTFETCH_TARGET_DIR_ROOT);

    if(ffStrbufIgnCaseCompS(&ffDetectOS(instance)->id, "bedrock") == 0)
        getPackageCountsBedrock(instance, &baseDir, &counts);
    else
        getPackageCounts(instance, &baseDir, &counts);

    // If SQL failed, we can still try with librpm.
    // This is needed on openSUSE, which seems to use a proprietary database file
    // This method doesn't work on bedrock, so we do it here.
    #ifdef FF_HAVE_RPM
        if(counts.rpm == 0)
            counts.rpm = getRpmFromLibrpm(instance);
    #endif

    //nix user
    ffStrbufSetS(&baseDir, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&baseDir, "/.nix-profile");
    uint32_t nixUser = getNixPackages(baseDir.chars);

    ffStrbufDestroy(&baseDir);

    uint32_t all = counts.pacman + counts.dpkg + counts.rpm + counts.emerge  + counts.xbps + counts.nixSystem + nixUser + counts.nixDefault + counts.apk + counts.flatpak + counts.snap;
    if(all == 0)
    {
        ffPrintError(instance, FF_PACKAGES_MODULE_NAME, 0, &instance->config.packages, "No packages from known package managers found");
        return;
    }

    if(instance->config.packages.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PACKAGES_MODULE_NAME, 0, &instance->config.packages.key);

        #define FF_PRINT_PACKAGE(name) \
        if(counts.name > 0) \
        { \
            printf("%u ("#name")", counts.name); \
            if((all = all - counts.name) > 0) \
                printf(", "); \
        };

        if(counts.pacman > 0)
        {
            printf("%u (pacman)", counts.pacman);
            if(counts.pacmanBranch.length > 0)
                printf("[%s]", counts.pacmanBranch.chars);
            if((all = all - counts.pacman) > 0)
                printf(", ");
        };

        FF_PRINT_PACKAGE(dpkg)
        FF_PRINT_PACKAGE(rpm)
        FF_PRINT_PACKAGE(emerge)
        FF_PRINT_PACKAGE(xbps)

        if(counts.nixSystem > 0)
        {
            printf("%u (nix-system)", counts.nixSystem);
            if((all = all - counts.nixSystem) > 0)
                printf(", ");
        }

        if(nixUser > 0)
        {
            printf("%u (nix-user)", nixUser);
            if((all = all - nixUser) > 0)
                printf(", ");
        }

        if(counts.nixDefault > 0)
        {
            printf("%u (nix-default)", counts.nixDefault);
            if((all = all - counts.nixDefault) > 0)
                printf(", ");
        }

        FF_PRINT_PACKAGE(apk)
        FF_PRINT_PACKAGE(flatpak)
        FF_PRINT_PACKAGE(snap)

        //Fix linter warning of unused value of all
        (void) all;

        #undef FF_PRINT_PACKAGE

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_PACKAGES_MODULE_NAME, 0, &instance->config.packages, FF_PACKAGES_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT, &all},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.pacman},
            {FF_FORMAT_ARG_TYPE_STRBUF, &counts.pacmanBranch},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.dpkg},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.rpm},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.emerge},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.xbps},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.nixSystem},
            {FF_FORMAT_ARG_TYPE_UINT, &nixUser},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.nixDefault},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.apk},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.flatpak},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.snap}
        });
    }

    ffStrbufDestroy(&counts.pacmanBranch);
}
