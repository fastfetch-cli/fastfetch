#include "fastfetch.h"
#include "common/printing.h"
#include "detection/packages/packages.h"

#define FF_PACKAGES_MODULE_NAME "Packages"
#define FF_PACKAGES_NUM_FORMAT_ARGS 17

void ffPrintPackages(FFinstance* instance)
{
    FFPackageCounts counts = {0};
    ffStrbufInit(&counts.pacmanBranch);
    ffDetectPackages(instance, &counts);

    uint32_t all = counts.pacman + counts.dpkg + counts.rpm + counts.emerge + counts.xbps + counts.nixSystem + counts.nixUser + counts.nixDefault + counts.apk + counts.pkg + counts.flatpak + counts.snap + counts.brew + counts.port + counts.scoop;
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

        if(counts.nixUser > 0)
        {
            printf("%u (nix-user)", counts.nixUser);
            if((all = all - counts.nixUser) > 0)
                printf(", ");
        }

        if(counts.nixDefault > 0)
        {
            printf("%u (nix-default)", counts.nixDefault);
            if((all = all - counts.nixDefault) > 0)
                printf(", ");
        }

        FF_PRINT_PACKAGE(apk)
        FF_PRINT_PACKAGE(pkg)
        FF_PRINT_PACKAGE(flatpak)
        FF_PRINT_PACKAGE(snap)
        FF_PRINT_PACKAGE(brew)
        FF_PRINT_PACKAGE(port)
        FF_PRINT_PACKAGE(scoop)

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
            {FF_FORMAT_ARG_TYPE_UINT, &counts.nixUser},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.nixDefault},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.apk},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.pkg},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.flatpak},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.snap},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.brew},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.port},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.scoop}
        });
    }

    ffStrbufDestroy(&counts.pacmanBranch);
}
