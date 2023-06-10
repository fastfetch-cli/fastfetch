#include "fastfetch.h"
#include "common/printing.h"
#include "detection/packages/packages.h"
#include "modules/packages/packages.h"

#define FF_PACKAGES_NUM_FORMAT_ARGS 21

void ffPrintPackages(FFinstance* instance, FFPackagesOptions* options)
{
    const FFPackagesResult* counts = ffDetectPackages(instance);
    uint32_t all = counts->all; //Copy it, so we can substract from it in FF_PRINT_PACKAGE

    if(all == 0)
    {
        ffPrintError(instance, FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, "No packages from known package managers found");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs.key);

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

        //Fix linter warning of unused value of all
        (void) all;

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, FF_PACKAGES_NUM_FORMAT_ARGS, (FFformatarg[]){
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
        });
    }
}

void ffInitPackagesOptions(FFPackagesOptions* options)
{
    options->moduleName = FF_PACKAGES_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParsePackagesCommandOptions(FFPackagesOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_PACKAGES_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyPackagesOptions(FFPackagesOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

#ifdef FF_HAVE_JSONC
void ffParsePackagesJsonObject(FFinstance* instance, json_object* module)
{
    FFPackagesOptions __attribute__((__cleanup__(ffDestroyPackagesOptions))) options;
    ffInitPackagesOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_PACKAGES_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintPackages(instance, &options);
}
#endif
