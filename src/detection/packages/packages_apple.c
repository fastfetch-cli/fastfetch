#include "packages.h"
#include "common/io/io.h"
#include "common/parsing.h"
#include "common/processing.h"
#include "util/stringUtils.h"

static uint32_t getNumElements(const char* dirname, unsigned char type)
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

    if(type == DT_DIR)
        num_elements -= 2; // accounting for . and ..

    return num_elements;
}

static void countBrewPackages(const char* dirname, FFPackagesResult* result)
{
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateS(dirname);

    uint32_t baseDirLength = baseDir.length;

    ffStrbufAppendS(&baseDir, "/Caskroom");
    result->brewCask += getNumElements(baseDir.chars, DT_DIR);
    ffStrbufSubstrBefore(&baseDir, baseDirLength);

    ffStrbufAppendS(&baseDir, "/Cellar");
    result->brew += getNumElements(baseDir.chars, DT_DIR);
    ffStrbufSubstrBefore(&baseDir, baseDirLength);
}

static void getBrewPackages(FFPackagesResult* result)
{
    const char* prefix = getenv("HOMEBREW_PREFIX");
    if(ffStrSet(prefix))
        return countBrewPackages(prefix, result);

    countBrewPackages(FASTFETCH_TARGET_DIR_ROOT "/opt/homebrew", result);
    countBrewPackages(FASTFETCH_TARGET_DIR_USR "/local", result);
}

static uint32_t countMacPortsPackages(const char* dirname)
{
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateS(dirname);
    ffStrbufAppendS(&baseDir, FASTFETCH_TARGET_DIR_ROOT "/var/macports/software");

    return getNumElements(baseDir.chars, DT_DIR);
}

static uint32_t getMacPortsPackages()
{
    const char* prefix = getenv("MACPORTS_PREFIX");
    if(ffStrSet(prefix))
        return countMacPortsPackages(prefix);

    return countMacPortsPackages(FASTFETCH_TARGET_DIR_ROOT "/opt/local");
}

static uint32_t getNixPackagesImpl(const char* path)
{
    //Nix detection is kinda slow, so we only do it if the dir exists
    if(!ffPathExists(path, FF_PATHTYPE_DIRECTORY))
        return 0;

    FF_STRBUF_AUTO_DESTROY output = ffStrbufCreateA(128);

    //https://github.com/fastfetch-cli/fastfetch/issues/195#issuecomment-1191748222
    FF_STRBUF_AUTO_DESTROY command = ffStrbufCreateA(255);
    ffStrbufAppendS(&command, "for x in $(nix-store --query --requisites ");
    ffStrbufAppendS(&command, path);
    ffStrbufAppendS(&command, "); do if [ -d $x ]; then echo $x ; fi ; done | cut -d- -f2- | egrep '([0-9]{1,}\\.)+[0-9]{1,}' | egrep -v '\\-doc$|\\-man$|\\-info$|\\-dev$|\\-bin$|^nixos-system-nixos-' | uniq | wc -l");

    ffProcessAppendStdOut(&output, (char* const[]) {
        FASTFETCH_TARGET_DIR_ROOT "/bin/sh",
        "-c",
        command.chars,
        NULL
    });

    return (uint32_t) strtoul(output.chars, NULL, 10);
}

static uint32_t getNixPackages(FFstrbuf* baseDir, const char* dirname)
{
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, dirname);
    uint32_t num_elements = getNixPackagesImpl(baseDir->chars);
    ffStrbufSubstrBefore(baseDir, baseDirLength);
    return num_elements;
}

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options)
{
    if (!(options->disabled & FF_PACKAGES_FLAG_BREW_BIT)) getBrewPackages(result);
    if (!(options->disabled & FF_PACKAGES_FLAG_MACPORTS_BIT)) result->macports = getMacPortsPackages();
    if (!(options->disabled & FF_PACKAGES_FLAG_NIX_BIT))
    {
        FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateS(FASTFETCH_TARGET_DIR_ROOT);
        result->nixDefault += getNixPackages(&baseDir, "/nix/var/nix/profiles/default");
        result->nixSystem += getNixPackages(&baseDir, "/run/current-system");
        ffStrbufSet(&baseDir, &instance.state.platform.homeDir);
        result->nixUser = getNixPackages(&baseDir, "/.nix-profile");
    }
}
