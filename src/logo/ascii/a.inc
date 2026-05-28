#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo A[] = {
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
    // AerOS
    {
        .names = { "aerOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_AEROS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    // Aeon
    {
        .names = { "Aeon" },
        .lines = FASTFETCH_DATATEXT_LOGO_AEON,
        .colors = {
            FF_COLOR_FG_256 "36",
            FF_COLOR_FG_256 "36",
        },
    },
    // AerynOS
    {
        .names = { "AerynOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_AERYNOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
        },
    },
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
    // Amazon
    {
        .names = { "Amazon" },
        .lines = FASTFETCH_DATATEXT_LOGO_AMAZON,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    // AmazonLinux
    {
        .names = { "Amazon Linux", "amzn" },
        .lines = FASTFETCH_DATATEXT_LOGO_AMAZON_LINUX,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_256 "178",
        },
    },
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
    // Arch
    {
        .names = { "arch", "archmerge" },
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
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
    // AxOS
    {
        .names = { "AxOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_AXOS,
        .colors = {
            FF_COLOR_FG_RGB "222;6;255",
            FF_COLOR_FG_RGB "222;6;255",
        },
    },
    // Azos
    {
        .names = { "Azos" },
        .lines = FASTFETCH_DATATEXT_LOGO_AZOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_RED,
        },
    },
    // LAST
    {},
};
