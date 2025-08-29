#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/packages/packages.h"
#include "modules/packages/packages.h"
#include "util/stringUtils.h"

bool ffPrintPackages(FFPackagesOptions* options)
{
    FFPackagesResult counts = {};
    ffStrbufInit(&counts.pacmanBranch);

    const char* error = ffDetectPackages(&counts, options);

    if(error)
    {
        ffPrintError(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    uint32_t nixAll = counts.nixDefault + counts.nixSystem + counts.nixUser;
    uint32_t flatpakAll = counts.flatpakSystem + counts.flatpakUser;
    uint32_t brewAll = counts.brew + counts.brewCask;
    uint32_t guixAll = counts.guixSystem + counts.guixUser + counts.guixHome;
    uint32_t hpkgAll = counts.hpkgSystem + counts.hpkgUser;
    uint32_t amAll = counts.amSystem + counts.amUser;
    uint32_t scoopAll = counts.scoopUser + counts.scoopGlobal;

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        #define FF_PRINT_PACKAGE_NAME(var, name) {\
            if(counts.var > 0) \
            { \
                printf("%u (%s)", counts.var, (name)); \
                if((all -= counts.var) > 0) \
                    fputs(", ", stdout); \
            } \
        }

        #define FF_PRINT_PACKAGE(name) FF_PRINT_PACKAGE_NAME(name, #name)

        #define FF_PRINT_PACKAGE_ALL(name) {\
            if(name ## All > 0) \
            { \
                printf("%u (%s)", name ## All, #name); \
                if((all -= name ## All) > 0) \
                    fputs(", ", stdout); \
            } \
        }

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
        if (options->combined)
        {
            FF_PRINT_PACKAGE_ALL(nix);
        }
        else
        {
            FF_PRINT_PACKAGE_NAME(nixSystem, "nix-system")
            FF_PRINT_PACKAGE_NAME(nixUser, "nix-user")
            FF_PRINT_PACKAGE_NAME(nixDefault, "nix-default")
        }
        FF_PRINT_PACKAGE(apk)
        FF_PRINT_PACKAGE(pkg)
        FF_PRINT_PACKAGE(pkgsrc)
        if (options->combined)
        {
            FF_PRINT_PACKAGE_ALL(hpkg)
        }
        else
        {
            FF_PRINT_PACKAGE_NAME(hpkgSystem, counts.hpkgUser ? "hpkg-system" : "hpkg")
            FF_PRINT_PACKAGE_NAME(hpkgUser, "hpkg-user")
        }
        if (options->combined)
        {
            FF_PRINT_PACKAGE_ALL(flatpak);
        }
        else
        {
            FF_PRINT_PACKAGE_NAME(flatpakSystem, counts.flatpakUser ? "flatpak-system" : "flatpak")
            FF_PRINT_PACKAGE_NAME(flatpakUser, "flatpak-user")
        }
        FF_PRINT_PACKAGE(snap)
        if (options->combined)
        {
            FF_PRINT_PACKAGE_ALL(brew);
        }
        else
        {
            FF_PRINT_PACKAGE_NAME(brew, "brew")
            FF_PRINT_PACKAGE_NAME(brewCask, "brew-cask")
        }
        FF_PRINT_PACKAGE(macports)
        if (options->combined)
        {
            FF_PRINT_PACKAGE_ALL(scoop);
        }
        else
        {
            FF_PRINT_PACKAGE_NAME(scoopUser, counts.scoopGlobal ? "scoop-user" : "scoop")
            FF_PRINT_PACKAGE_NAME(scoopGlobal, "scoop-global")
        }
        FF_PRINT_PACKAGE(choco)
        FF_PRINT_PACKAGE(pkgtool)
        FF_PRINT_PACKAGE(paludis)
        FF_PRINT_PACKAGE(winget)
        FF_PRINT_PACKAGE(opkg)
        if (options->combined)
        {
            FF_PRINT_PACKAGE_ALL(am);
        }
        else
        {
            FF_PRINT_PACKAGE_NAME(amSystem, "am")
            FF_PRINT_PACKAGE_NAME(amUser, "appman")
        }
        FF_PRINT_PACKAGE(sorcery)
        FF_PRINT_PACKAGE(lpkg)
        FF_PRINT_PACKAGE(lpkgbuild)
        if (options->combined)
        {
            FF_PRINT_PACKAGE_ALL(guix);
        }
        else
        {
            FF_PRINT_PACKAGE_NAME(guixSystem, "guix-system")
            FF_PRINT_PACKAGE_NAME(guixUser, "guix-user")
            FF_PRINT_PACKAGE_NAME(guixHome, "guix-home")
        }
        FF_PRINT_PACKAGE(linglong)
        FF_PRINT_PACKAGE(pacstall)
        FF_PRINT_PACKAGE(mport)
        FF_PRINT_PACKAGE(pisi)
        FF_PRINT_PACKAGE(soar)

        putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
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
            FF_FORMAT_ARG(counts.scoopUser, "scoop-user"),
            FF_FORMAT_ARG(counts.scoopGlobal, "scoop-global"),
            FF_FORMAT_ARG(counts.choco, "choco"),
            FF_FORMAT_ARG(counts.pkgtool, "pkgtool"),
            FF_FORMAT_ARG(counts.paludis, "paludis"),
            FF_FORMAT_ARG(counts.winget, "winget"),
            FF_FORMAT_ARG(counts.opkg, "opkg"),
            FF_FORMAT_ARG(counts.amSystem, "am-system"),
            FF_FORMAT_ARG(counts.sorcery, "sorcery"),
            FF_FORMAT_ARG(counts.lpkg, "lpkg"),
            FF_FORMAT_ARG(counts.lpkgbuild, "lpkgbuild"),
            FF_FORMAT_ARG(counts.guixSystem, "guix-system"),
            FF_FORMAT_ARG(counts.guixUser, "guix-user"),
            FF_FORMAT_ARG(counts.guixHome, "guix-home"),
            FF_FORMAT_ARG(counts.linglong, "linglong"),
            FF_FORMAT_ARG(counts.pacstall, "pacstall"),
            FF_FORMAT_ARG(counts.mport, "mport"),
            FF_FORMAT_ARG(counts.amUser, "am-user"),
            FF_FORMAT_ARG(counts.pkgsrc, "pkgsrc"),
            FF_FORMAT_ARG(counts.hpkgSystem, "hpkg-system"),
            FF_FORMAT_ARG(counts.hpkgUser, "hpkg-user"),
            FF_FORMAT_ARG(counts.pisi, "pisi"),
            FF_FORMAT_ARG(counts.soar, "soar"),
            FF_FORMAT_ARG(nixAll, "nix-all"),
            FF_FORMAT_ARG(flatpakAll, "flatpak-all"),
            FF_FORMAT_ARG(brewAll, "brew-all"),
            FF_FORMAT_ARG(guixAll, "guix-all"),
            FF_FORMAT_ARG(hpkgAll, "hpkg-all"),
        }));
    }

    ffStrbufDestroy(&counts.pacmanBranch);

    return true;
}

void ffParsePackagesJsonObject(FFPackagesOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "disabled"))
        {
            if (!yyjson_is_null(val) && !yyjson_is_arr(val))
            {
                ffPrintError(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid JSON value for %s", unsafe_yyjson_get_str(key));
                continue;
            }

            options->disabled = FF_PACKAGES_FLAG_NONE;

            if (yyjson_is_arr(val))
            {
                yyjson_val* flagObj;
                size_t flagIdx, flagMax;
                yyjson_arr_foreach(val, flagIdx, flagMax, flagObj)
                {
                    if (!yyjson_is_str(flagObj))
                    {
                        ffPrintError(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid JSON value for %s", unsafe_yyjson_get_str(key));
                        continue;
                    }
                    const char* flag = unsafe_yyjson_get_str(flagObj);

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
                        case 'H': if (false);
                            FF_TEST_PACKAGE_NAME(HPKG)
                            break;
                        case 'L': if (false);
                            FF_TEST_PACKAGE_NAME(LPKG)
                            FF_TEST_PACKAGE_NAME(LPKGBUILD)
                            FF_TEST_PACKAGE_NAME(LINGLONG)
                            break;
                        case 'M': if (false);
                            FF_TEST_PACKAGE_NAME(MACPORTS)
                            FF_TEST_PACKAGE_NAME(MPORT)
                            break;
                        case 'N': if (false);
                            FF_TEST_PACKAGE_NAME(NIX)
                            break;
                        case 'O': if (false);
                            FF_TEST_PACKAGE_NAME(OPKG)
                            break;
                        case 'P': if (false);
                            FF_TEST_PACKAGE_NAME(PACMAN)
                            FF_TEST_PACKAGE_NAME(PACSTALL)
                            FF_TEST_PACKAGE_NAME(PALUDIS)
                            FF_TEST_PACKAGE_NAME(PISI)
                            FF_TEST_PACKAGE_NAME(PKG)
                            FF_TEST_PACKAGE_NAME(PKGTOOL)
                            FF_TEST_PACKAGE_NAME(PKGSRC)
                            break;
                        case 'R': if (false);
                            FF_TEST_PACKAGE_NAME(RPM)
                            break;
                        case 'S': if (false);
                            FF_TEST_PACKAGE_NAME(SCOOP)
                            FF_TEST_PACKAGE_NAME(SNAP)
                            FF_TEST_PACKAGE_NAME(SOAR)
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

        if (unsafe_yyjson_equals_str(key, "combined"))
        {
            options->combined = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_PACKAGES_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGeneratePackagesJsonConfig(FFPackagesOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "disabled");
    #define FF_TEST_PACKAGE_NAME(name) else if ((options->disabled & FF_PACKAGES_FLAG_ ## name ## _BIT)) { \
        ffStrbufSetS(&buf, #name); \
        ffStrbufLowerCase(&buf); \
        yyjson_mut_arr_add_strbuf(doc, arr, &buf); \
    }
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
    FF_TEST_PACKAGE_NAME(HPKG)
    FF_TEST_PACKAGE_NAME(LINGLONG)
    FF_TEST_PACKAGE_NAME(LPKG)
    FF_TEST_PACKAGE_NAME(LPKGBUILD)
    FF_TEST_PACKAGE_NAME(MACPORTS)
    FF_TEST_PACKAGE_NAME(MPORT)
    FF_TEST_PACKAGE_NAME(NIX)
    FF_TEST_PACKAGE_NAME(OPKG)
    FF_TEST_PACKAGE_NAME(PACMAN)
    FF_TEST_PACKAGE_NAME(PACSTALL)
    FF_TEST_PACKAGE_NAME(PALUDIS)
    FF_TEST_PACKAGE_NAME(PISI)
    FF_TEST_PACKAGE_NAME(PKG)
    FF_TEST_PACKAGE_NAME(PKGTOOL)
    FF_TEST_PACKAGE_NAME(PKGSRC)
    FF_TEST_PACKAGE_NAME(RPM)
    FF_TEST_PACKAGE_NAME(SCOOP)
    FF_TEST_PACKAGE_NAME(SNAP)
    FF_TEST_PACKAGE_NAME(SOAR)
    FF_TEST_PACKAGE_NAME(SORCERY)
    FF_TEST_PACKAGE_NAME(WINGET)
    FF_TEST_PACKAGE_NAME(XBPS)
    #undef FF_TEST_PACKAGE_NAME

    yyjson_mut_obj_add_bool(doc, module, "combined", options->combined);
}

bool ffGeneratePackagesJsonResult(FF_MAYBE_UNUSED FFPackagesOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFPackagesResult counts = {};
    ffStrbufInit(&counts.pacmanBranch);

    const char* error = ffDetectPackages(&counts, options);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");

    #define FF_APPEND_PACKAGE_COUNT(name) yyjson_mut_obj_add_uint(doc, obj, #name, counts.name);

    FF_APPEND_PACKAGE_COUNT(all)
    FF_APPEND_PACKAGE_COUNT(amSystem)
    FF_APPEND_PACKAGE_COUNT(amUser)
    FF_APPEND_PACKAGE_COUNT(apk)
    FF_APPEND_PACKAGE_COUNT(brew)
    FF_APPEND_PACKAGE_COUNT(brewCask)
    FF_APPEND_PACKAGE_COUNT(choco)
    FF_APPEND_PACKAGE_COUNT(dpkg)
    FF_APPEND_PACKAGE_COUNT(emerge)
    FF_APPEND_PACKAGE_COUNT(eopkg)
    FF_APPEND_PACKAGE_COUNT(flatpakSystem)
    FF_APPEND_PACKAGE_COUNT(flatpakUser)
    FF_APPEND_PACKAGE_COUNT(guixSystem)
    FF_APPEND_PACKAGE_COUNT(guixUser)
    FF_APPEND_PACKAGE_COUNT(guixHome)
    FF_APPEND_PACKAGE_COUNT(hpkgSystem)
    FF_APPEND_PACKAGE_COUNT(hpkgUser)
    FF_APPEND_PACKAGE_COUNT(linglong)
    FF_APPEND_PACKAGE_COUNT(mport)
    FF_APPEND_PACKAGE_COUNT(nixDefault)
    FF_APPEND_PACKAGE_COUNT(nixSystem)
    FF_APPEND_PACKAGE_COUNT(nixUser)
    FF_APPEND_PACKAGE_COUNT(opkg)
    FF_APPEND_PACKAGE_COUNT(pacman)
    FF_APPEND_PACKAGE_COUNT(pacstall)
    FF_APPEND_PACKAGE_COUNT(paludis)
    FF_APPEND_PACKAGE_COUNT(pisi)
    FF_APPEND_PACKAGE_COUNT(pkg)
    FF_APPEND_PACKAGE_COUNT(pkgtool)
    FF_APPEND_PACKAGE_COUNT(pkgsrc)
    FF_APPEND_PACKAGE_COUNT(macports)
    FF_APPEND_PACKAGE_COUNT(rpm)
    FF_APPEND_PACKAGE_COUNT(scoopUser)
    FF_APPEND_PACKAGE_COUNT(scoopGlobal)
    FF_APPEND_PACKAGE_COUNT(snap)
    FF_APPEND_PACKAGE_COUNT(soar)
    FF_APPEND_PACKAGE_COUNT(sorcery)
    FF_APPEND_PACKAGE_COUNT(winget)
    FF_APPEND_PACKAGE_COUNT(xbps)
    yyjson_mut_obj_add_strbuf(doc, obj, "pacmanBranch", &counts.pacmanBranch);

    return true;
}

void ffInitPackagesOptions(FFPackagesOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰏖");

    options->disabled = FF_PACKAGES_DISABLE_LIST;
    options->combined = false;
}

void ffDestroyPackagesOptions(FFPackagesOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffPackagesModuleInfo = {
    .name = FF_PACKAGES_MODULE_NAME,
    .description = "List installed package managers and count of installed packages",
    .initOptions = (void*) ffInitPackagesOptions,
    .destroyOptions = (void*) ffDestroyPackagesOptions,
    .parseJsonObject = (void*) ffParsePackagesJsonObject,
    .printModule = (void*) ffPrintPackages,
    .generateJsonResult = (void*) ffGeneratePackagesJsonResult,
    .generateJsonConfig = (void*) ffGeneratePackagesJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Number of all packages", "all"},
        {"Number of pacman packages", "pacman"},
        {"Pacman branch on manjaro", "pacman-branch"},
        {"Number of dpkg packages", "dpkg"},
        {"Number of rpm packages", "rpm"},
        {"Number of emerge packages", "emerge"},
        {"Number of eopkg packages", "eopkg"},
        {"Number of xbps packages", "xbps"},
        {"Number of nix-system packages", "nix-system"},
        {"Number of nix-user packages", "nix-user"},
        {"Number of nix-default packages", "nix-default"},
        {"Number of apk packages", "apk"},
        {"Number of pkg packages", "pkg"},
        {"Number of flatpak-system app packages", "flatpak-system"},
        {"Number of flatpak-user app packages", "flatpak-user"},
        {"Number of snap packages", "snap"},
        {"Number of brew packages", "brew"},
        {"Number of brew-cask packages", "brew-cask"},
        {"Number of macports packages", "macports"},
        {"Number of scoop-user packages", "scoop-user"},
        {"Number of scoop-global packages", "scoop-global"},
        {"Number of choco packages", "choco"},
        {"Number of pkgtool packages", "pkgtool"},
        {"Number of paludis packages", "paludis"},
        {"Number of winget packages", "winget"},
        {"Number of opkg packages", "opkg"},
        {"Number of am-system packages", "am-system"},
        {"Number of sorcery packages", "sorcery"},
        {"Number of lpkg packages", "lpkg"},
        {"Number of lpkgbuild packages", "lpkgbuild"},
        {"Number of guix-system packages", "guix-system"},
        {"Number of guix-user packages", "guix-user"},
        {"Number of guix-home packages", "guix-home"},
        {"Number of linglong packages", "linglong"},
        {"Number of pacstall packages", "pacstall"},
        {"Number of mport packages", "mport"},
        {"Number of am-user (aka appman) packages", "am-user"},
        {"Number of pkgsrc packages", "pkgsrc"},
        {"Number of hpkg-system packages", "hpkg-system"},
        {"Number of hpkg-user packages", "hpkg-user"},
        {"Number of pisi packages", "pisi"},
        {"Number of soar packages", "soar"},
        {"Total number of all nix packages", "nix-all"},
        {"Total number of all flatpak app packages", "flatpak-all"},
        {"Total number of all brew packages", "brew-all"},
        {"Total number of all guix packages", "guix-all"},
        {"Total number of all hpkg packages", "hpkg-all"},
    }))
};
