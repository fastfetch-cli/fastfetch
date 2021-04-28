#include "fastfetch.h"

#include <dirent.h>

#define FF_PACKAGES_MODULE_NAME "Packages"
#define FF_PACKAGES_NUM_FORMAT_ARGS 3

enum elementType
{
    enumDir = 0,
    enumFile
};

static uint32_t get_num_elements(const char* dirname, enum elementType type) {
    uint32_t num_elements = 0;
    DIR * dirp;
    struct dirent *entry;

    dirp = opendir(dirname);
    if(dirp == NULL)
        return 0;

    while((entry = readdir(dirp)) != NULL)
    {
        if(entry->d_type == DT_DIR && type == enumDir)
            ++num_elements;
        else if(entry->d_type == DT_REG && type == enumFile)
            ++num_elements;
    }

    if(type == enumDir)
        num_elements -= 2; // accounting for . and ..

    closedir(dirp);

    return num_elements;
}

void ffPrintPackages(FFinstance* instance)
{
    uint32_t pacman = get_num_elements("/var/lib/pacman/local", enumDir);
    uint32_t flatpak = get_num_elements("/var/lib/flatpak/app", enumDir);
    uint32_t xbps = get_num_elements("/var/db/xbps", enumFile);

    uint32_t all = pacman + flatpak + xbps;

    if(all == 0)
    {
        ffPrintError(instance, FF_PACKAGES_MODULE_NAME, 0, &instance->config.packagesKey, &instance->config.packagesFormat, FF_PACKAGES_NUM_FORMAT_ARGS, "No packages from known package managers found");
        return;
    }

    if(instance->config.packagesFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PACKAGES_MODULE_NAME, 0, &instance->config.batteryKey);

        #define FF_PRINT_PACKAGE(name) \
        if(name > 0) \
        { \
            printf("%u ("#name")", name); \
            if((all = all - name) > 0) \
                printf(", "); \
        };

        FF_PRINT_PACKAGE(pacman)
        FF_PRINT_PACKAGE(flatpak)
        FF_PRINT_PACKAGE(xbps)

        #undef FF_PRINT_PACKAGE

        putchar('\n');
    }
    else
    {
        ffPrintFormatString(instance, FF_PACKAGES_MODULE_NAME, 0, &instance->config.packagesKey, &instance->config.packagesFormat, NULL, FF_PACKAGES_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT, &all},
            {FF_FORMAT_ARG_TYPE_UINT, &pacman},
            {FF_FORMAT_ARG_TYPE_UINT, &flatpak},
            {FF_FORMAT_ARG_TYPE_UINT, &xbps}
        });
    }
}
