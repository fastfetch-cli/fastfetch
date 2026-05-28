#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo A[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_ADELIE
    // Adélie
    {
        .names = { "Adélie", "Adelie" },
        .lines = FASTFETCH_DATATEXT_LOGO_ADELIE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AEROS
    // AerOS
    {
        .names = { "aerOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_AEROS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AEON
    // Aeon
    {
        .names = { "Aeon" },
        .lines = FASTFETCH_DATATEXT_LOGO_AEON,
        .colors = {
            FF_COLOR_FG_256 "36",
            FF_COLOR_FG_256 "36",
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AERYNOS
    // AerynOS
    {
        .names = { "AerynOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_AERYNOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AERYNOS_OLD
    // AerynOS_old
    {
        .names = { "AerynOS_old" },
        .lines = FASTFETCH_DATATEXT_LOGO_AERYNOS_OLD,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_DEFAULT,
            FF_COLOR_FG_MAGENTA,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AFTERGLOW
    // Afterglow
    {
        .names = { "Afterglow" },
        .lines = FASTFETCH_DATATEXT_LOGO_AFTERGLOW,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AIX
    // AIX
    {
        .names = { "aix" },
        .lines = FASTFETCH_DATATEXT_LOGO_AIX,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ALMALINUX
    // AlmaLinux
    {
        .names = { "Almalinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_ALMALINUX,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_LIGHT_YELLOW,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_LIGHT_GREEN,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ALPINE
    // Alpine
    {
        .names = { "Alpine" },
        .lines = FASTFETCH_DATATEXT_LOGO_ALPINE,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ALPINE2
    // Alpine2
    {
        .names = { "Alpine2" },
        .lines = FASTFETCH_DATATEXT_LOGO_ALPINE2,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ALPINE_SMALL
    // AlpineSmall
    {
        .names = { "Alpine_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ALPINE_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ALPINE2_SMALL
    // Alpine2Small
    {
        .names = { "alpine2_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ALPINE2_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ALPINE3_SMALL
    // Alpine3Small
    {
        .names = { "alpine3_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ALPINE3_SMALL,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ALTER
    // Alter
    {
        .names = { "Alter" },
        .lines = FASTFETCH_DATATEXT_LOGO_ALTER,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ALTLINUX
    // ALTLinux
    {
        .names = { "ALTLinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_ALTLINUX,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AMAZON
    // Amazon
    {
        .names = { "Amazon" },
        .lines = FASTFETCH_DATATEXT_LOGO_AMAZON,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AMAZON_LINUX
    // AmazonLinux
    {
        .names = { "Amazon Linux", "amzn" },
        .lines = FASTFETCH_DATATEXT_LOGO_AMAZON_LINUX,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_256 "178",
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AMIGA
    // Amiga
    {
        .names = { "Amiga" },
        .lines = FASTFETCH_DATATEXT_LOGO_AMIGA,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_LIGHT_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_LIGHT_YELLOW,
            FF_COLOR_FG_GREEN,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AMOGOS
    // AmogOS
    {
        .names = { "AmogOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_AMOGOS,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ANARCHY
    // Anarchy
    {
        .names = { "Anarchy" },
        .lines = FASTFETCH_DATATEXT_LOGO_ANARCHY,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ANDROID
    // Android
    {
        .names = { "android" },
        .lines = FASTFETCH_DATATEXT_LOGO_ANDROID,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ANDROID_SMALL
    // AndroidSmall
    {
        .names = { "android_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ANDROID_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ANDUINOS
    // AnduinOS
    {
        .names = { "anduinos" },
        .lines = FASTFETCH_DATATEXT_LOGO_ANDUINOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ANTERGOS
    // Antergos
    {
        .names = { "Antergos" },
        .lines = FASTFETCH_DATATEXT_LOGO_ANTERGOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ANTIX
    // Antix
    {
        .names = { "antiX" },
        .lines = FASTFETCH_DATATEXT_LOGO_ANTIX,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ANUSHOS
    // AnushOS
    {
        .names = { "AnushOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_ANUSHOS,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AOSCOSRETRO
    // AoscOsRetro
    {
        .names = { "Aosc OS/Retro", "aoscosretro" },
        .lines = FASTFETCH_DATATEXT_LOGO_AOSCOSRETRO,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AOSCOSRETRO_SMALL
    // AoscOsRetro_small
    {
        .names = { "Aosc OS/Retro_small", "aoscosretro_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_AOSCOSRETRO_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AOSCOS
    // AoscOS
    {
        .names = { "Aosc OS", "aoscos" },
        .lines = FASTFETCH_DATATEXT_LOGO_AOSCOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AOSCOS_OLD
    // AoscOS_old
    {
        .names = { "Aosc OS_old", "aoscos_old" },
        .lines = FASTFETCH_DATATEXT_LOGO_AOSCOS_OLD,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_APERTURE
    // Aperture
    {
        .names = { "Aperture" },
        .lines = FASTFETCH_DATATEXT_LOGO_APERTURE,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_MACOS
    // Apple
    {
        .names = { "Apple" },
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_LIGHT_RED,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_MACOS_SMALL
    // AppleSmall
    {
        .names = { "Apple_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_APRICITY
    // Apricity
    {
        .names = { "Apricity" },
        .lines = FASTFETCH_DATATEXT_LOGO_APRICITY,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARCHBOX
    // ArchBox
    {
        .names = { "ArchBox" },
        .lines = FASTFETCH_DATATEXT_LOGO_ARCHBOX,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARCHCRAFT
    // Archcraft
    {
        .names = { "Archcraft" },
        .lines = FASTFETCH_DATATEXT_LOGO_ARCHCRAFT,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARCHCRAFT2
    // Archcraft2
    {
        .names = { "Archcraft2" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARCHCRAFT2,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARCH
    // Arch
    {
        .names = { "arch", "archmerge" },
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARCH2
    // Arch2
    {
        .names = { "arch2" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH2,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARCH3
    // Arch3
    {
        .names = { "arch3" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH3,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARCH_SMALL
    // ArchSmall
    {
        .names = { "arch_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARCH_OLD
    // ArchOld
    {
        .names = { "arch_old" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH_OLD,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorTitle = FF_COLOR_FG_DEFAULT,
        .colorKeys = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARCHLABS
    // Archlabs
    {
        .names = { "ARCHlabs" },
        .lines = FASTFETCH_DATATEXT_LOGO_ARCHLABS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARCHSTRIKE
    // ArchStrike
    {
        .names = { "ArchStrike" },
        .lines = FASTFETCH_DATATEXT_LOGO_ARCHSTRIKE,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARKANE
    // Arkane
    {
        .names = { "Arkane", "Arkane Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_ARKANE,
        .colors = {
            FF_COLOR_FG_256 "237",
            FF_COLOR_FG_256 "130",
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_256 "130",
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARMBIAN
    // Armbian
    {
        .names = { "Armbian" },
        .lines = FASTFETCH_DATATEXT_LOGO_ARMBIAN,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARMBIAN2
    // Armbian2
    {
        .names = { "Armbian2" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARMBIAN2,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARTIX
    // Artix
    {
        .names = { "artix" },
        .lines = FASTFETCH_DATATEXT_LOGO_ARTIX,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARTIX_SMALL
    // ArtixSmall
    {
        .names = { "artix_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARTIX_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARTIX2_SMALL
    // Artix2Small
    {
        .names = { "artix2_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARTIX2_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARCO
    // ArcoLinux (Discontinued)
    {
        .names = { "arco", "arcolinux" }, // ID=arcolinux
        .lines = FASTFETCH_DATATEXT_LOGO_ARCO,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARCO_SMALL
    // ArcoLinuxSmall
    {
        .names = { "arco_small", "arcolinux_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARCO_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARSELINUX
    // ArseLinux
    {
        .names = { "arse", "arselinux", "arse-linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_ARSELINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ARYA
    // Arya
    {
        .names = { "Arya" },
        .lines = FASTFETCH_DATATEXT_LOGO_ARYA,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ASAHI
    // Asahi
    {
        .names = { "asahi", "asahi-linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_ASAHI,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ASAHI2
    // Asahi2
    {
        .names = { "asahi2", "asahi-linux2" },
        .lines = FASTFETCH_DATATEXT_LOGO_ASAHI2,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_LIGHT_YELLOW,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_LIGHT_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_LIGHT_CYAN,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ASTER
    // Aster
    {
        .names = { "aster" },
        .lines = FASTFETCH_DATATEXT_LOGO_ASTER,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ASTEROIDOS
    // AsteroidOS
    {
        .names = { "AsteroidOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_ASTEROIDOS,
        .colors = {
            FF_COLOR_FG_256 "160",
            FF_COLOR_FG_256 "208",
            FF_COLOR_FG_256 "202",
            FF_COLOR_FG_256 "214",
        },
        .colorKeys = FF_COLOR_FG_256 "160",
        .colorTitle = FF_COLOR_FG_256 "208",
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ASTOS
    // AstOS
    {
        .names = { "astOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_ASTOS,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ASTRA_LINUX
    // Astra
    {
        .names = { "Astra", "Astra Linux", "astralinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_ASTRA_LINUX,
        .colors = {
            FF_COLOR_FG_LIGHT_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_JANUSLINUX
    // Ataraxia
    {
        .names = { "Ataraxia Linux", "Ataraxia" },
        .lines = FASTFETCH_DATATEXT_LOGO_JANUSLINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ATHENAOS
    // AthenaOS
    {
        .names = { "AthenaOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_ATHENAOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_LIGHT_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_LIGHT_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ATHENAOS_OLD
    // AthenaOS_old
    {
        .names = { "AthenaOS_old" },
        .lines = FASTFETCH_DATATEXT_LOGO_ATHENAOS_OLD,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AURORA
    // Aurora
    {
        .names = { "Aurora" },
        .lines = FASTFETCH_DATATEXT_LOGO_AURORA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AXOS
    // AxOS
    {
        .names = { "AxOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_AXOS,
        .colors = {
            FF_COLOR_FG_RGB "222;6;255",
            FF_COLOR_FG_RGB "222;6;255",
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_AZOS
    // Azos
    {
        .names = { "Azos" },
        .lines = FASTFETCH_DATATEXT_LOGO_AZOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_RED,
        },
    },
    #endif
    // LAST
    {},
};
