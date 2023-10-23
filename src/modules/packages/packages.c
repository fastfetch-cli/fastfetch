#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/packages/packages.h"
#include "modules/packages/packages.h"
#include "util/stringUtils.h"

#define FF_PACKAGES_NUM_FORMAT_ARGS 25

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
        FF_PRINT_PACKAGE(winget)
        FF_PRINT_PACKAGE(opkg)

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
            {FF_FORMAT_ARG_TYPE_UINT, &counts.paludis},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.winget},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.opkg},
        });
    }

    ffStrbufDestroy(&counts.pacmanBranch);
}

bool ffParsePackagesCommandOptions(FFPackagesOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_PACKAGES_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
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

void ffGeneratePackagesJsonResult(FF_MAYBE_UNUSED FFPackagesOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFPackagesResult counts = {};
    ffStrbufInit(&counts.pacmanBranch);

    const char* error = ffDetectPackages(&counts);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");

    #define FF_APPEND_PACKAGE_COUNT(name) yyjson_mut_obj_add_uint(doc, obj, #name, counts.name);

    FF_APPEND_PACKAGE_COUNT(all)
    FF_APPEND_PACKAGE_COUNT(apk)
    FF_APPEND_PACKAGE_COUNT(brew)
    FF_APPEND_PACKAGE_COUNT(brewCask)
    FF_APPEND_PACKAGE_COUNT(choco)
    FF_APPEND_PACKAGE_COUNT(dpkg)
    FF_APPEND_PACKAGE_COUNT(emerge)
    FF_APPEND_PACKAGE_COUNT(eopkg)
    FF_APPEND_PACKAGE_COUNT(flatpakSystem)
    FF_APPEND_PACKAGE_COUNT(flatpakUser)
    FF_APPEND_PACKAGE_COUNT(nixDefault)
    FF_APPEND_PACKAGE_COUNT(nixSystem)
    FF_APPEND_PACKAGE_COUNT(nixUser)
    FF_APPEND_PACKAGE_COUNT(pacman)
    FF_APPEND_PACKAGE_COUNT(paludis)
    FF_APPEND_PACKAGE_COUNT(pkg)
    FF_APPEND_PACKAGE_COUNT(pkgtool)
    FF_APPEND_PACKAGE_COUNT(port)
    FF_APPEND_PACKAGE_COUNT(rpm)
    FF_APPEND_PACKAGE_COUNT(scoop)
    FF_APPEND_PACKAGE_COUNT(snap)
    FF_APPEND_PACKAGE_COUNT(winget)
    FF_APPEND_PACKAGE_COUNT(xbps)
    FF_APPEND_PACKAGE_COUNT(opkg)
    yyjson_mut_obj_add_strbuf(doc, obj, "pacmanBranch", &counts.pacmanBranch);
}

void ffPrintPackagesHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_PACKAGES_MODULE_NAME, "{2} (pacman){?3}[{3}]{?}, {4} (dpkg), {5} (rpm), {6} (emerge), {7} (eopkg), {8} (xbps), {9} (nix-system), {10} (nix-user), {11} (nix-default), {12} (apk), {13} (pkg), {14} (flatpak-system), {15} (flatpack-user), {16} (snap), {17} (brew), {18} (brew-cask), {19} (port), {20} (scoop), {21} (choco), {22} (pkgtool), {23} (paludis), {24} (winget), {25} (opkg)", FF_PACKAGES_NUM_FORMAT_ARGS, (const char* []) {
        "Number of all packages",
        "Number of pacman packages",
        "Pacman branch on manjaro",
        "Number of dpkg packages",
        "Number of rpm packages",
        "Number of emerge packages",
        "Number of eopkg packages",
        "Number of xbps packages",
        "Number of nix-system packages",
        "Number of nix-user packages",
        "Number of nix-default packages",
        "Number of apk packages",
        "Number of pkg packages",
        "Number of flatpak-system packages",
        "Number of flatpak-user packages",
        "Number of snap packages",
        "Number of brew packages",
        "Number of brew-cask packages",
        "Number of macports packages",
        "Number of scoop packages",
        "Number of choco packages",
        "Number of pkgtool packages",
        "Number of paludis packages",
        "Number of winget packages",
        "Number of opkg packages"
    });
}

void ffInitPackagesOptions(FFPackagesOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_PACKAGES_MODULE_NAME, ffParsePackagesCommandOptions, ffParsePackagesJsonObject, ffPrintPackages, ffGeneratePackagesJsonResult, ffPrintPackagesHelpFormat);
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyPackagesOptions(FFPackagesOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
