#include "packages.h"
#include "common/parsing.h"
#include "util/stringUtils.h"

#include <dirent.h>

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

    countBrewPackages(FASTFETCH_TARGET_DIR_ROOT"/opt/homebrew", result);
    countBrewPackages(FASTFETCH_TARGET_DIR_ROOT"/usr/local", result);
}

static uint32_t countMacPortsPackages(const char* dirname)
{
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateS(dirname);
    ffStrbufAppendS(&baseDir, "/var/macports/software");

    return getNumElements(baseDir.chars, DT_DIR);
}

static uint32_t getMacPortsPackages()
{
    const char* prefix = getenv("MACPORTS_PREFIX");
    if(ffStrSet(prefix))
        return countMacPortsPackages(prefix);

    return countMacPortsPackages(FASTFETCH_TARGET_DIR_ROOT"/opt/local");
}

void ffDetectPackagesImpl(const FFinstance* instance, FFPackagesResult* result)
{
    FF_UNUSED(instance);
    getBrewPackages(result);
    result->port = getMacPortsPackages();
}
