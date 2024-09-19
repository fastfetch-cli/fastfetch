#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/packages/packages.h"
#include "modules/packages/packages.h"
#include "util/stringUtils.h"

#define FF_PACKAGES_NUM_FORMAT_ARGS 37

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
        FF_PRINT_PACKAGE(lpkg)
        FF_PRINT_PACKAGE(lpkgbuild)
        FF_PRINT_PACKAGE_NAME(guixSystem, "guix-system")
        FF_PRINT_PACKAGE_NAME(guixUser, "guix-user")
        FF_PRINT_PACKAGE_NAME(guixHome, "guix-home")
        FF_PRINT_PACKAGE(linglong)

        putchar('\n');
    }
    else
    {
        uint32_t nixAll = counts.nixDefault + counts.nixSystem + counts.nixUser;
        uint32_t flatpakAll = counts.flatpakSystem + counts.flatpakUser;
        uint32_t brewAll = counts.brew + counts.brewCask;
        uint32_t guixAll = counts.guixSystem + counts.guixUser + counts.guixHome;
        FF_PRINT_FORMAT_CHECKED(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_PACKAGES_NUM_FORMAT_ARGS, ((FFformatarg[]){
            FF_FORMAT_ARG(counts.all, "all"),
            FF_FORMAT_ARG(counts.pacman, "pacman"),
            FF_FORMAT_ARG(counts.pacmanBranch, "pacman-branch"),
            FF_FORMAT_ARG(counts.dpkg, "dpkg"),
            FF_FORMAT_ARG(counts.rpm, "rpm"),
            FF_FORMAT_ARG(counts.emerge, "emerge"),
            FF_FORMAT_ARG(counts.eopkg, "eopkg"),
            FF_FORMAT_ARG(counts.xbps, "xbps"),
            FF_FORMAT_ARG(counts.nixSystem, "nix-system"),
            FF_FORMAT_ARG(counts.nixUser, "nix-user"),
            FF_FORMAT_ARG(counts.nixDefault, "nix-default"),
            FF_FORMAT_ARG(counts.apk, "apk"),
            FF_FORMAT_ARG(counts.pkg, "pkg"),
            FF_FORMAT_ARG(counts.flatpakSystem, "flatpak-system"),
            FF_FORMAT_ARG(counts.flatpakUser, "flatpak-user"),
            FF_FORMAT_ARG(counts.snap, "snap"),
            FF_FORMAT_ARG(counts.brew, "brew"),
            FF_FORMAT_ARG(counts.brewCask, "brew-cask"),
            FF_FORMAT_ARG(counts.macports, "macports"),
            FF_FORMAT_ARG(counts.scoop, "scoop"),
            FF_FORMAT_ARG(counts.choco, "choco"),
            FF_FORMAT_ARG(counts.pkgtool, "pkgtool"),
            FF_FORMAT_ARG(counts.paludis, "paludis"),
            FF_FORMAT_ARG(counts.winget, "winget"),
            FF_FORMAT_ARG(counts.opkg, "opkg"),
            FF_FORMAT_ARG(counts.am, "am"),
            FF_FORMAT_ARG(counts.sorcery, "sorcery"),
            FF_FORMAT_ARG(counts.lpkg, "lpkg"),
            FF_FORMAT_ARG(counts.lpkgbuild, "lpkgbuild"),
            FF_FORMAT_ARG(counts.guixSystem, "guix-system"),
            FF_FORMAT_ARG(counts.guixUser, "guix-user"),
            FF_FORMAT_ARG(counts.guixHome, "guix-home"),
            FF_FORMAT_ARG(counts.linglong, "linglong"),
            FF_FORMAT_ARG(nixAll, "nix-all"),
            FF_FORMAT_ARG(flatpakAll, "flatpak-all"),
            FF_FORMAT_ARG(brewAll, "brew-all"),
            FF_FORMAT_ARG(guixAll, "guix-all"),
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
                case 'G': if (false);
                    FF_TEST_PACKAGE_NAME(GUIX)
                    break;
                case 'L': if (false);
                    FF_TEST_PACKAGE_NAME(LPKG)
                    FF_TEST_PACKAGE_NAME(LPKGBUILD)
                    FF_TEST_PACKAGE_NAME(LINGLONG)
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
                       case 'G': if (false);
                            FF_TEST_PACKAGE_NAME(GUIX)
                            break;
                        case 'L': if (false);
                            FF_TEST_PACKAGE_NAME(LPKG)
                            FF_TEST_PACKAGE_NAME(LPKGBUILD)
                            FF_TEST_PACKAGE_NAME(LINGLONG)
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
        FF_TEST_PACKAGE_NAME(AM)
        FF_TEST_PACKAGE_NAME(APK)
        FF_TEST_PACKAGE_NAME(BREW)
        FF_TEST_PACKAGE_NAME(CHOCO)
        FF_TEST_PACKAGE_NAME(DPKG)
        FF_TEST_PACKAGE_NAME(EMERGE)
        FF_TEST_PACKAGE_NAME(EOPKG)
        FF_TEST_PACKAGE_NAME(FLATPAK)
        FF_TEST_PACKAGE_NAME(GUIX)
        FF_TEST_PACKAGE_NAME(LINGLONG)
        FF_TEST_PACKAGE_NAME(LPKG)
        FF_TEST_PACKAGE_NAME(LPKGBUILD)
        FF_TEST_PACKAGE_NAME(MACPORTS)
        FF_TEST_PACKAGE_NAME(NIX)
        FF_TEST_PACKAGE_NAME(OPKG)
        FF_TEST_PACKAGE_NAME(PACMAN)
        FF_TEST_PACKAGE_NAME(PALUDIS)
        FF_TEST_PACKAGE_NAME(PKG)
        FF_TEST_PACKAGE_NAME(PKGTOOL)
        FF_TEST_PACKAGE_NAME(RPM)
        FF_TEST_PACKAGE_NAME(SCOOP)
        FF_TEST_PACKAGE_NAME(SNAP)
        FF_TEST_PACKAGE_NAME(SORCERY)
        FF_TEST_PACKAGE_NAME(WINGET)
        FF_TEST_PACKAGE_NAME(XBPS)
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
    FF_APPEND_PACKAGE_COUNT(guixSystem)
    FF_APPEND_PACKAGE_COUNT(guixUser)
    FF_APPEND_PACKAGE_COUNT(guixHome)
    FF_APPEND_PACKAGE_COUNT(linglong)
    yyjson_mut_obj_add_strbuf(doc, obj, "pacmanBranch", &counts.pacmanBranch);
}

void ffPrintPackagesHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_PACKAGES_MODULE_NAME, "{2} (pacman){?3}[{3}]{?}, {4} (dpkg), {5} (rpm), {6} (emerge), {7} (eopkg), {8} (xbps), {9} (nix-system), {10} (nix-user), {11} (nix-default), {12} (apk), {13} (pkg), {14} (flatpak-system), {15} (flatpack-user), {16} (snap), {17} (brew), {18} (brew-cask), {19} (MacPorts), {20} (scoop), {21} (choco), {22} (pkgtool), {23} (paludis), {24} (winget), {25} (opkg), {26} (am), {27} (sorcery), {28} (lpkg), {29} (lpkgbuild), {30} (guix-system), {31} (guix-user), {32} (guix-home), {33} (linglong)", FF_PACKAGES_NUM_FORMAT_ARGS, ((const char* []) {
        "Number of all packages - all",
        "Number of pacman packages - pacman",
        "Pacman branch on manjaro - pacman-branch",
        "Number of dpkg packages - dpkg",
        "Number of rpm packages - rpm",
        "Number of emerge packages - emerge",
        "Number of eopkg packages - eopkg",
        "Number of xbps packages - xbps",
        "Number of nix-system packages - nix-system",
        "Number of nix-user packages - nix-user",
        "Number of nix-default packages - nix-default",
        "Number of apk packages - apk",
        "Number of pkg packages - pkg",
        "Number of flatpak-system app packages - flatpak-system",
        "Number of flatpak-user app packages - flatpak-user",
        "Number of snap packages - snap",
        "Number of brew packages - brew",
        "Number of brew-cask packages - brew-cask",
        "Number of macports packages - macports",
        "Number of scoop packages - scoop",
        "Number of choco packages - choco",
        "Number of pkgtool packages - pkgtool",
        "Number of paludis packages - paludis",
        "Number of winget packages - winget",
        "Number of opkg packages - opkg",
        "Number of am packages - am",
        "Number of sorcery packages - sorcery",
        "Number of lpkg packages - lpkg",
        "Number of lpkgbuild packages - lpkgbuild",
        "Number of guix-system packages - guix-system",
        "Number of guix-user packages - guix-user",
        "Number of guix-home packages - guix-home",
        "Number of linglong packages - linglong",
        "Total number of all nix packages - nix-all",
        "Total number of all flatpak app packages - flatpak-all",
        "Total number of all brew packages - brew-all",
        "Total number of all guix packages - guix-all",
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
    ffOptionInitModuleArg(&options->moduleArgs, "󰏖");

    options->disabled = FF_PACKAGES_DISABLE_LIST;
}

void ffDestroyPackagesOptions(FFPackagesOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
