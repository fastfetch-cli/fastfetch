#include "packages.h"
#include "common/parsing.h"

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

static uint32_t countBrewPackages(const char* dirname)
{
    FFstrbuf baseDir;
    ffStrbufInitS(&baseDir, dirname);

    uint32_t result = 0;
    uint32_t baseDirLength = baseDir.length;

    ffStrbufAppendS(&baseDir, "/Caskroom");
    result += getNumElements(baseDir.chars, DT_DIR);
    ffStrbufSubstrBefore(&baseDir, baseDirLength);

    ffStrbufAppendS(&baseDir, "/Cellar");
    result += getNumElements(baseDir.chars, DT_DIR);
    ffStrbufSubstrBefore(&baseDir, baseDirLength);

    ffStrbufDestroy(&baseDir);
    return result;
}

static uint32_t getBrewPackages()
{
    const char* prefix = getenv("HOMEBREW_PREFIX");
    if(ffStrSet(prefix))
        return countBrewPackages(prefix);

    uint32_t result = 0;
    result += countBrewPackages(FASTFETCH_TARGET_DIR_ROOT"/opt/homebrew");
    result += countBrewPackages(FASTFETCH_TARGET_DIR_ROOT"/usr/local");
    return result;
}

static uint32_t countMacPortsPackages(const char* dirname)
{
    FFstrbuf baseDir;
    ffStrbufInitS(&baseDir, dirname);
    ffStrbufAppendS(&baseDir, "/var/macports/software");

    uint32_t result = getNumElements(baseDir.chars, DT_DIR);

    ffStrbufDestroy(&baseDir);
    return result;
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
    result->brew = getBrewPackages();
    result->port = getMacPortsPackages();
}
