#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef enum FFPackagesFlags
{
    FF_PACKAGES_FLAG_NONE = 0,
    FF_PACKAGES_FLAG_APK_BIT = 1 << 0,
    FF_PACKAGES_FLAG_BREW_BIT = 1 << 1,
    FF_PACKAGES_FLAG_CHOCO_BIT = 1 << 2,
    FF_PACKAGES_FLAG_DPKG_BIT = 1 << 3,
    FF_PACKAGES_FLAG_EMERGE_BIT = 1 << 4,
    FF_PACKAGES_FLAG_EOPKG_BIT = 1 << 5,
    FF_PACKAGES_FLAG_FLATPAK_BIT = 1 << 6,
    FF_PACKAGES_FLAG_NIX_BIT = 1 << 7,
    FF_PACKAGES_FLAG_OPKG_BIT = 1 << 8,
    FF_PACKAGES_FLAG_PACMAN_BIT = 1 << 9,
    FF_PACKAGES_FLAG_PALUDIS_BIT = 1 << 10,
    FF_PACKAGES_FLAG_PKG_BIT = 1 << 11,
    FF_PACKAGES_FLAG_PKGTOOL_BIT = 1 << 12,
    FF_PACKAGES_FLAG_MACPORTS_BIT = 1 << 13,
    FF_PACKAGES_FLAG_RPM_BIT = 1 << 14,
    FF_PACKAGES_FLAG_SCOOP_BIT = 1 << 15,
    FF_PACKAGES_FLAG_SNAP_BIT = 1 << 16,
    FF_PACKAGES_FLAG_WINGET_BIT = 1 << 17,
    FF_PACKAGES_FLAG_XBPS_BIT = 1 << 18,
    FF_PACKAGES_FLAG_AM_BIT = 1 << 19,
    FF_PACKAGES_FLAG_SORCERY_BIT = 1 << 20,
    FF_PACKAGES_FLAG_LPKG_BIT = 1 << 21,
    FF_PACKAGES_FLAG_LPKGBUILD_BIT = 1 << 22,
    FF_PACKAGES_FLAG_GUIX_BIT = 1 << 23,
    FF_PACKAGES_FLAG_LINGLONG_BIT = 1 << 24,
    FF_PACKAGES_FLAG_PACSTALL_BIT = 1 << 25,
} FFPackagesFlags;

typedef struct FFPackagesOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

    FFPackagesFlags disabled;
} FFPackagesOptions;
