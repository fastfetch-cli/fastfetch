#include "packages.h"

#include "common/io/io.h"

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

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options)
{
    if (!(options->disabled & FF_PACKAGES_FLAG_PKG_BIT))
    {
        result->pkg = getNumElementsImpl(FASTFETCH_TARGET_DIR_ROOT "/var/db/pkg", DT_DIR);
    }
}
