#include "fastfetch.h"
#include "common/printing.h"
#include "detection/packages/packages.h"

#define FF_PACKAGES_MODULE_NAME "Packages"
#define FF_PACKAGES_NUM_FORMAT_ARGS 22

void ffPrintPackages(FFinstance* instance)
{
    const FFPackagesResult* counts = ffDetectPackages(instance);
    uint32_t all = counts->all; //Copy it, so we can substract from it in FF_PRINT_PACKAGE

    if(all == 0)
    {
        ffPrintError(instance, FF_PACKAGES_MODULE_NAME, 0, &instance->config.packages, "No packages from known package managers found");
        return;
    }

    if(instance->config.packages.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PACKAGES_MODULE_NAME, 0, &instance->config.packages.key);

        #define FF_PRINT_PACKAGE_NAME(var, name) \
            if(counts->var > 0) \
            { \
                printf("%u (%s)", counts->var, (name)); \
                if((all -= counts->var) > 0) \
                    printf(", "); \
            };

        #define FF_PRINT_PACKAGE(name) FF_PRINT_PACKAGE_NAME(name, #name)

        if(counts->pacman > 0)
        {
            printf("%u (pacman)", counts->pacman);
            if(counts->pacmanBranch.length > 0)
                printf("[%s]", counts->pacmanBranch.chars);
            if((all -= counts->pacman) > 0)
                printf(", ");
        };

        FF_PRINT_PACKAGE(dpkg)
        FF_PRINT_PACKAGE(rpm)
        FF_PRINT_PACKAGE(emerge)
        FF_PRINT_PACKAGE(eopkg)
        FF_PRINT_PACKAGE(xbps)
        FF_PRINT_PACKAGE_NAME(nixSystem, "nix-system")
        FF_PRINT_PACKAGE_NAME(nixUser, "nix-user")
        FF_PRINT_PACKAGE_NAME(nixDefault, "nix-default")
        FF_PRINT_PACKAGE(apk)
        FF_PRINT_PACKAGE(pkg)
        FF_PRINT_PACKAGE_NAME(flatpakSystem, counts->flatpakUser ? "flatpak-system" : "flatpak")
        FF_PRINT_PACKAGE_NAME(flatpakUser, "flatpak-user")
        FF_PRINT_PACKAGE(snap)
        FF_PRINT_PACKAGE(brew)
        FF_PRINT_PACKAGE_NAME(brewCask, "brew-cask")
        FF_PRINT_PACKAGE(port)
        FF_PRINT_PACKAGE(scoop)
        FF_PRINT_PACKAGE(choco)
        FF_PRINT_PACKAGE(pkgtool)

        //Fix linter warning of unused value of all
        (void) all;

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_PACKAGES_MODULE_NAME, 0, &instance->config.packages, FF_PACKAGES_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT, &all},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->pacman},
            {FF_FORMAT_ARG_TYPE_STRBUF, &counts->pacmanBranch},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->dpkg},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->rpm},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->emerge},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->eopkg},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->xbps},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->nixSystem},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->nixUser},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->nixDefault},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->apk},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->pkg},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->flatpakSystem},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->flatpakUser},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->snap},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->brew},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->brewCask},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->port},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->scoop},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->choco},
            {FF_FORMAT_ARG_TYPE_UINT, &counts->pkgtool},
        });
    }
}
