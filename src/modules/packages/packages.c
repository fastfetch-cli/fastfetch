#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/packages/packages.h"
#include "modules/packages/packages.h"
#include "util/stringUtils.h"

#define FF_PACKAGES_NUM_FORMAT_ARGS 30

void ffPrintPackages(FFPackagesOptions* options)
{
    FFPackagesResult counts = {};
    ffStrbufInit(&counts.pacmanBranch);

    const char* error = ffDetectPackages(&counts, options);

    if(error)
    {
        ffPrintError(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
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
                    fputs(", ", stdout); \
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
        FF_PRINT_PACKAGE(macports)
        FF_PRINT_PACKAGE(scoop)
        FF_PRINT_PACKAGE(choco)
        FF_PRINT_PACKAGE(pkgtool)
        FF_PRINT_PACKAGE(paludis)
        FF_PRINT_PACKAGE(winget)
        FF_PRINT_PACKAGE(opkg)
        FF_PRINT_PACKAGE(am)
        FF_PRINT_PACKAGE(sorcery)

        putchar('\n');
    }
    else
    {
        uint32_t nixAll = counts.nixDefault + counts.nixSystem + counts.nixUser;
        uint32_t flatpakAll = counts.flatpakSystem + counts.flatpakUser;
        uint32_t brewAll = counts.brew + counts.brewCask;
        FF_PRINT_FORMAT_CHECKED(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_PACKAGES_NUM_FORMAT_ARGS, ((FFformatarg[]){
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
            {FF_FORMAT_ARG_TYPE_UINT, &counts.macports},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.scoop},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.choco},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.pkgtool},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.paludis},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.winget},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.opkg},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.am},
            {FF_FORMAT_ARG_TYPE_UINT, &counts.sorcery},
            {FF_FORMAT_ARG_TYPE_UINT, &nixAll},
            {FF_FORMAT_ARG_TYPE_UINT, &flatpakAll},
            {FF_FORMAT_ARG_TYPE_UINT, &brewAll},
        }));
    }

    ffStrbufDestroy(&counts.pacmanBranch);
}

bool ffParsePackagesCommandOptions(FFPackagesOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_PACKAGES_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "disabled"))
    {
        options->disabled = FF_PACKAGES_FLAG_NONE;
        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
        ffOptionParseString(key, value, &buffer);

        char *start = buffer.chars, *end = strchr(start, ':');
        while (true)
        {
            if (end)
                *end = '\0';

            #define FF_TEST_PACKAGE_NAME(name) else if (ffStrEqualsIgnCase(start, #name)) { options->disabled |= FF_PACKAGES_FLAG_ ## name ## _BIT; }
            switch (toupper(start[0]))
            {
                case 'A': if (false);
                    FF_TEST_PACKAGE_NAME(APK)
                    FF_TEST_PACKAGE_NAME(AM)
                    break;
                case 'B': if (false);
                    FF_TEST_PACKAGE_NAME(BREW)
                    break;
                case 'C': if (false);
                    FF_TEST_PACKAGE_NAME(CHOCO)
                    break;
                case 'D': if (false);
                    FF_TEST_PACKAGE_NAME(DPKG)
                    break;
                case 'E': if (false);
                    FF_TEST_PACKAGE_NAME(EMERGE)
                    FF_TEST_PACKAGE_NAME(EOPKG)
                    break;
                case 'F': if (false);
                    FF_TEST_PACKAGE_NAME(FLATPAK)
                    break;
                case 'M': if (false);
                    FF_TEST_PACKAGE_NAME(MACPORTS)
                    break;
                case 'N': if (false);
                    FF_TEST_PACKAGE_NAME(NIX)
                    break;
                case 'O': if (false);
                    FF_TEST_PACKAGE_NAME(OPKG)
                    break;
                case 'P': if (false);
                    FF_TEST_PACKAGE_NAME(PACMAN)
                    FF_TEST_PACKAGE_NAME(PKG)
                    FF_TEST_PACKAGE_NAME(PKGTOOL)
                    FF_TEST_PACKAGE_NAME(PALUDIS)
                    break;
                case 'R': if (false);
                    FF_TEST_PACKAGE_NAME(RPM)
                    break;
                case 'S': if (false);
                    FF_TEST_PACKAGE_NAME(SCOOP)
                    FF_TEST_PACKAGE_NAME(SNAP)
                    FF_TEST_PACKAGE_NAME(SORCERY)
                    break;
                case 'W': if (false);
                    FF_TEST_PACKAGE_NAME(WINGET)
                    break;
                case 'X': if (false);
                    FF_TEST_PACKAGE_NAME(XBPS)
                    break;
            }
            #undef FF_TEST_PACKAGE_NAME

            if (end)
            {
                start = end + 1;
                end = strchr(start, ':');
            }
            else
                break;
        }

        return true;
    }

    return false;
}

void ffParsePackagesJsonObject(FFPackagesOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if (ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (ffStrEqualsIgnCase(key, "disabled"))
        {
            if (!yyjson_is_null(val) && !yyjson_is_arr(val))
            {
                ffPrintError(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid JSON value for %s", key);
                continue;
            }

            options->disabled = FF_PACKAGES_FLAG_NONE;

            if (yyjson_is_arr(val))
            {
                yyjson_val* flagObj;
                size_t flagIdx, flagMax;
                yyjson_arr_foreach(val, flagIdx, flagMax, flagObj)
                {
                    const char* flag = yyjson_get_str(flagObj);

                    #define FF_TEST_PACKAGE_NAME(name) else if (ffStrEqualsIgnCase(flag, #name)) { options->disabled |= FF_PACKAGES_FLAG_ ## name ## _BIT; }
                    switch (toupper(flag[0]))
                    {
                        case 'A': if (false);
                            FF_TEST_PACKAGE_NAME(APK)
                            FF_TEST_PACKAGE_NAME(AM)
                            break;
                        case 'B': if (false);
                            FF_TEST_PACKAGE_NAME(BREW)
                            break;
                        case 'C': if (false);
                            FF_TEST_PACKAGE_NAME(CHOCO)
                            break;
                        case 'D': if (false);
                            FF_TEST_PACKAGE_NAME(DPKG)
                            break;
                        case 'E': if (false);
                            FF_TEST_PACKAGE_NAME(EMERGE)
                            FF_TEST_PACKAGE_NAME(EOPKG)
                            break;
                        case 'F': if (false);
                            FF_TEST_PACKAGE_NAME(FLATPAK)
                            break;
                        case 'M': if (false);
                            FF_TEST_PACKAGE_NAME(MACPORTS)
                            break;
                        case 'N': if (false);
                            FF_TEST_PACKAGE_NAME(NIX)
                            break;
                        case 'O': if (false);
                            FF_TEST_PACKAGE_NAME(OPKG)
                            break;
                        case 'P': if (false);
                            FF_TEST_PACKAGE_NAME(PACMAN)
                            FF_TEST_PACKAGE_NAME(PKG)
                            FF_TEST_PACKAGE_NAME(PKGTOOL)
                            FF_TEST_PACKAGE_NAME(PALUDIS)
                            break;
                        case 'R': if (false);
                            FF_TEST_PACKAGE_NAME(RPM)
                            break;
                        case 'S': if (false);
                            FF_TEST_PACKAGE_NAME(SCOOP)
                            FF_TEST_PACKAGE_NAME(SNAP)
                            FF_TEST_PACKAGE_NAME(SORCERY)
                            break;
                        case 'W': if (false);
                            FF_TEST_PACKAGE_NAME(WINGET)
                            break;
                        case 'X': if (false);
                            FF_TEST_PACKAGE_NAME(XBPS)
                            break;
                    }
                    #undef FF_TEST_PACKAGE_NAME
                }
                continue;
            }
        }

        ffPrintError(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGeneratePackagesJsonConfig(FFPackagesOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyPackagesOptions))) FFPackagesOptions defaultOptions;
    ffInitPackagesOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (options->disabled != defaultOptions.disabled)
    {
        yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "disabled");
        #define FF_TEST_PACKAGE_NAME(name) else if ((options->disabled & FF_PACKAGES_FLAG_ ## name ## _BIT) != (defaultOptions.disabled & FF_PACKAGES_FLAG_ ## name ## _BIT)) { yyjson_mut_arr_add_str(doc, arr, #name); }
        if (false);
        FF_TEST_PACKAGE_NAME(APK)
        FF_TEST_PACKAGE_NAME(BREW)
        FF_TEST_PACKAGE_NAME(CHOCO)
        FF_TEST_PACKAGE_NAME(DPKG)
        FF_TEST_PACKAGE_NAME(EMERGE)
        FF_TEST_PACKAGE_NAME(EOPKG)
        FF_TEST_PACKAGE_NAME(FLATPAK)
        FF_TEST_PACKAGE_NAME(NIX)
        FF_TEST_PACKAGE_NAME(OPKG)
        FF_TEST_PACKAGE_NAME(PACMAN)
        FF_TEST_PACKAGE_NAME(PALUDIS)
        FF_TEST_PACKAGE_NAME(PKG)
        FF_TEST_PACKAGE_NAME(PKGTOOL)
        FF_TEST_PACKAGE_NAME(MACPORTS)
        FF_TEST_PACKAGE_NAME(RPM)
        FF_TEST_PACKAGE_NAME(SCOOP)
        FF_TEST_PACKAGE_NAME(SNAP)
        FF_TEST_PACKAGE_NAME(WINGET)
        FF_TEST_PACKAGE_NAME(XBPS)
        FF_TEST_PACKAGE_NAME(AM)
        FF_TEST_PACKAGE_NAME(SORCERY)
        #undef FF_TEST_PACKAGE_NAME
    }
}

void ffGeneratePackagesJsonResult(FF_MAYBE_UNUSED FFPackagesOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFPackagesResult counts = {};
    ffStrbufInit(&counts.pacmanBranch);

    const char* error = ffDetectPackages(&counts, options);

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
    FF_APPEND_PACKAGE_COUNT(macports)
    FF_APPEND_PACKAGE_COUNT(rpm)
    FF_APPEND_PACKAGE_COUNT(scoop)
    FF_APPEND_PACKAGE_COUNT(snap)
    FF_APPEND_PACKAGE_COUNT(winget)
    FF_APPEND_PACKAGE_COUNT(xbps)
    FF_APPEND_PACKAGE_COUNT(opkg)
    FF_APPEND_PACKAGE_COUNT(am)
    FF_APPEND_PACKAGE_COUNT(sorcery)
    yyjson_mut_obj_add_strbuf(doc, obj, "pacmanBranch", &counts.pacmanBranch);
}

void ffPrintPackagesHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_PACKAGES_MODULE_NAME, "{2} (pacman){?3}[{3}]{?}, {4} (dpkg), {5} (rpm), {6} (emerge), {7} (eopkg), {8} (xbps), {9} (nix-system), {10} (nix-user), {11} (nix-default), {12} (apk), {13} (pkg), {14} (flatpak-system), {15} (flatpack-user), {16} (snap), {17} (brew), {18} (brew-cask), {19} (MacPorts), {20} (scoop), {21} (choco), {22} (pkgtool), {23} (paludis), {24} (winget), {25} (opkg), {26} (am), {27} (sorcery)", FF_PACKAGES_NUM_FORMAT_ARGS, ((const char* []) {
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
        "Number of opkg packages",
        "Number of am packages",
        "Number of sorcery packages",
        "Total number of all nix packages",
        "Total number of all flatpak packages",
        "Total number of all brew packages",
    }));
}

void ffInitPackagesOptions(FFPackagesOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_PACKAGES_MODULE_NAME,
        "List installed package managers and count of installed packages",
        ffParsePackagesCommandOptions,
        ffParsePackagesJsonObject,
        ffPrintPackages,
        ffGeneratePackagesJsonResult,
        ffPrintPackagesHelpFormat,
        ffGeneratePackagesJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);

    options->disabled = FF_PACKAGES_FLAG_WINGET_BIT;
}

void ffDestroyPackagesOptions(FFPackagesOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
