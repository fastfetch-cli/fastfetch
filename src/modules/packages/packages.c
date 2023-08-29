#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/packages/packages.h"
#include "modules/packages/packages.h"
#include "util/stringUtils.h"

#define FF_PACKAGES_NUM_FORMAT_ARGS 22

void ffPrintPackages(FFPackagesOptions* options)
{
    FFPackagesResult counts = {};
    ffStrbufInit(&counts.pacmanBranch);

    const char* error = ffDetectPackages(&counts);

    if(error)
    {
        ffPrintError(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        #define FF_PRINT_PACKAGE_NAME(var, name) \
            if(counts.var > 0) \
            { \
                printf("%u (%s)", counts.var, (name)); \
                if((all -= counts.var) > 0) \
                    printf(", "); \
            };

        #define FF_PRINT_PACKAGE(name) FF_PRINT_PACKAGE_NAME(name, #name)

        uint32_t all = counts.all;
        if(counts.pacman > 0)
        {
            printf("%u (pacman)", counts.pacman);
            if(counts.pacmanBranch.length > 0)
                printf("[%s]", counts.pacmanBranch.chars);
            if((all -= counts.pacman) > 0)
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
        FF_PRINT_PACKAGE_NAME(flatpakSystem, counts.flatpakUser ? "flatpak-system" : "flatpak")
        FF_PRINT_PACKAGE_NAME(flatpakUser, "flatpak-user")
        FF_PRINT_PACKAGE(snap)
        FF_PRINT_PACKAGE(brew)
        FF_PRINT_PACKAGE_NAME(brewCask, "brew-cask")
        FF_PRINT_PACKAGE(port)
        FF_PRINT_PACKAGE(scoop)
        FF_PRINT_PACKAGE(choco)
        FF_PRINT_PACKAGE(pkgtool)
        FF_PRINT_PACKAGE(paludis)

        putchar('\n');
    }
    else
    {
        ffPrintFormat(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, FF_PACKAGES_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT, &counts.all},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.pacman},
            {FF_FORMAT_ARG_TYPE_STRBUF, &counts.pacmanBranch},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.dpkg},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.rpm},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.emerge},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.eopkg},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.xbps},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.nixSystem},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.nixUser},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.nixDefault},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.apk},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.pkg},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.flatpakSystem},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.flatpakUser},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.snap},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.brew},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.brewCask},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.port},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.scoop},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.choco},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.pkgtool},
        });
    }

    ffStrbufDestroy(&counts.pacmanBranch);
}

void ffInitPackagesOptions(FFPackagesOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_PACKAGES_MODULE_NAME, ffParsePackagesCommandOptions, ffParsePackagesJsonObject, ffPrintPackages, NULL);
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

void ffParsePackagesJsonObject(FFPackagesOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}
