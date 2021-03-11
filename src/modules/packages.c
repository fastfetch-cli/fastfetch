#include "fastfetch.h"

#include <dirent.h>

static uint32_t get_num_dirs(const char* dirname) {
    uint32_t num_dirs = 0;
    DIR * dirp;
    struct dirent *entry;

    dirp = opendir(dirname);
    if(dirp == NULL)
        return 0;

    while((entry = readdir(dirp)) != NULL) {
        if(entry->d_type == DT_DIR)
            ++num_dirs;
    }
   
    num_dirs -= 2; // accounting for . and ..

    closedir(dirp);

    return num_dirs;
}

void ffPrintPackages(FFinstance* instance)
{
    uint32_t pacman = instance->config.packagesShowPacman ? get_num_dirs("/var/lib/pacman/local") : 0;
    uint32_t flatpak = instance->config.packagesShowFlatpak ? get_num_dirs("/var/lib/flatpak/app") : 0;

    uint32_t all = pacman + flatpak;

    if(all == 0)
    {
        ffPrintError(instance, "Packages", "No packages from known package managers found");
        return;
    }

    ffPrintLogoAndKey(instance, "Packages");

    if(instance->config.packagesFormat[0] != '\0')
    {
        printf(instance->config.packagesFormat, all, pacman, flatpak, all);
        putchar('\n');
        return;
    }

    if(instance->config.packagesCombined)
    {
        printf("%u", all);

        if(instance->config.packagesCombinedNames)
        {
            printf(" (");
            
            #define FF_PRINT_PACKAGE(name) \
                if(name > 0) \
                { \
                    printf(#name); \
                    if((all = all - name) > 0) \
                        printf(", "); \
                } \

            FF_PRINT_PACKAGE(pacman)
            FF_PRINT_PACKAGE(flatpak)

            #undef FF_PRINT_PACKAGE

            printf(")");
        }

        putchar('\n');
    }
    else
    {
        #define FF_PRINT_PACKAGE(name) \
            if(name > 0) \
            { \
                printf("%u ("#name")", name); \
                if((all = all - name) > 0) \
                    printf(", "); \
            };

        FF_PRINT_PACKAGE(pacman)
        FF_PRINT_PACKAGE(flatpak)

        #undef FF_PRINT_PACKAGE

        putchar('\n');
    }
}