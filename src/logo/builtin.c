#include "logo.h"
#include "logo_builtin.h"
#include "common/color.h"

const FFlogo ffLogoUnknown = {
    .names = {"unknown"},
    .lines = FASTFETCH_DATATEXT_LOGO_UNKNOWN,
    .colors = {
        FF_COLOR_FG_WHITE,
    },
    .colorKeys = "",
    .colorTitle = "",
};

static const FFlogo A[] = {
    // Adélie
    {
        .names = {"Adélie", "Adelie"},
        .lines = FASTFETCH_DATATEXT_LOGO_ADELIE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
        },
    },
    // AerOS
    {
        .names = {"aerOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_AEROS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    // Afterglow
    {
        .names = {"Afterglow"},
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
        .names = {"aix"},
        .lines = FASTFETCH_DATATEXT_LOGO_AIX,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // AlmaLinux
    {
        .names = {"almalinux"},
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
        .names = {"alpine", "alpinelinux", "alpine-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ALPINE,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // AlpineSmall
    {
        .names = {"alpine_small", "alpine-linux-small"},
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
        .names = {"alpine2_small", "alpine-linux2-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ALPINE2_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Alter
    {
        .names = {"Alter"},
        .lines = FASTFETCH_DATATEXT_LOGO_ALTER,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Amazon
    {
        .names = {"Amazon"},
        .lines = FASTFETCH_DATATEXT_LOGO_AMAZON,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        }
    },
    // AmazonLinux
    {
        .names = {"Amazon Linux", "amzn"},
        .lines = FASTFETCH_DATATEXT_LOGO_AMAZON_LINUX,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_256 "178",
        }
    },
    // AmogOS
    {
        .names = {"AmogOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_AMOGOS,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Anarchy
    {
        .names = {"Anarchy"},
        .lines = FASTFETCH_DATATEXT_LOGO_ANARCHY,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Android
    {
        .names = {"android"},
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
        .names = {"android-small", "android_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ANDROID_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Antergos
    {
        .names = {"Antergos"},
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
        .names = {"antiX"},
        .lines = FASTFETCH_DATATEXT_LOGO_ANTIX,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // AoscOsRetro
    {
        .names = {"Aosc OS/Retro", "aoscosretro"},
        .lines = FASTFETCH_DATATEXT_LOGO_AOSCOSRETRO,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // AoscOsRetro_small
    {
        .names = {"Aosc OS/Retro_small", "aoscosretro_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_AOSCOSRETRO_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // AoscOS
    {
        .names = {"Aosc OS", "aoscos"},
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
        .names = {"Aosc OS_old", "aoscos_old"},
        .lines = FASTFETCH_DATATEXT_LOGO_AOSCOS_OLD,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Aperture
    {
        .names = {"Aperture"},
        .lines = FASTFETCH_DATATEXT_LOGO_APERTURE,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Apple
    {
        .names = {"Apple"},
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS,
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
    // AppleSmall
    {
        .names = {"Apple_small"},
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
        .names = {"Apricity"},
        .lines = FASTFETCH_DATATEXT_LOGO_APRICITY,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // ArchBox
    {
        .names = {"ArchBox"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARCHBOX,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Archcraft
    {
        .names = {"Archcraft"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARCHCRAFT,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Archcraft2
    {
        .names = {"Archcraft2"},
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
        .names = {"arch", "archlinux", "arch-linux", "archmerge"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Arch2
    {
        .names = {"arch2", "archlinux2", "arch-linux2"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH2,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // ArchSmall
    {
        .names = {"arch_small", "archlinux_small", "arch-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Archlabs
    {
        .names = {"ARCHlabs"},
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
        .names = {"ArchStrike"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARCHSTRIKE,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Armbian
    {
        .names = {"Armbian"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARMBIAN,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
    },
    // Armbian2
    {
        .names = {"Armbian2"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARMBIAN2,
        .colors = {
            FF_COLOR_FG_RED,
        },
    },
    // Artix
    {
        .names = {"artix", "artixlinux", "artix-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARTIX,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // ArtixSmall
    {
        .names = {"artix_small", "artixlinux_small", "artix-linux-small"},
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
        .names = {"artix2_small", "artixlinux2_small", "artix-linux2-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARTIX2_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // ArcoLinux
    {
        .names = {"arco", "arcolinux", "arco-linux"},
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
        .names = {"arco_small", "arcolinux_small", "arco-linux_small"},
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
        .names = {"arse", "arselinux", "arse-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARSELINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Arya
    {
        .names = {"Arya"},
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
        .names = {"asahi", "asahi-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ASAHI,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
            "38", //cyan
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Asahi2
    {
        .names = {"asahi2", "asahi-linux2"},
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
        .names = {"aster"},
        .lines = FASTFETCH_DATATEXT_LOGO_ASTER,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // AsteroidOS
    {
        .names = {"AsteroidOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_ASTEROIDOS,
        .colors = {
            FF_COLOR_FG_256 "160",
            FF_COLOR_FG_256 "208",
            FF_COLOR_FG_256 "202",
            FF_COLOR_FG_256 "214"
        },
        .colorKeys = FF_COLOR_FG_256 "160",
        .colorTitle = FF_COLOR_FG_256 "208",
    },
    // AstOS
    {
        .names = {"astOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_ASTOS,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Astra
    {
        .names = {"Astra", "Astra Linux", "astralinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ASTRA_LINUX,
        .colors = {
            FF_COLOR_FG_LIGHT_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Ataraxia
    {
        .names = {"Ataraxia Linux", "Ataraxia"},
        .lines = FASTFETCH_DATATEXT_LOGO_JANUSLINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // Athena
    {
        .names = {"Athena"},
        .lines = FASTFETCH_DATATEXT_LOGO_ATHENA,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Azos
    {
        .names = {"Azos"},
        .lines = FASTFETCH_DATATEXT_LOGO_AZOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_RED,
        }
    },
    // LAST
    {},
};

static const FFlogo B[] = {
    // Bedrock
    {
        .names = {"bedrock", "bedrocklinux", "bedrock-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_BEDROCK,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK, //grey
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK, //grey
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // BigLinux
    {
        .names = {"BigLinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_BIGLINUX,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Bitrig
    {
        .names = {"Bitrig"},
        .lines = FASTFETCH_DATATEXT_LOGO_BITRIG,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // BlackArch
    {
        .names = {"Blackarch"},
        .lines = FASTFETCH_DATATEXT_LOGO_BLACKARCH,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_LIGHT_RED,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // BlackMesa
    {
        .names = {"BlackMesa", "black-mesa"},
        .lines = FASTFETCH_DATATEXT_LOGO_BLACKMESA,
        .colors = {
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_BLACK,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // BlackPanther
    {
        .names = {"BlackPanther"},
        .lines = FASTFETCH_DATATEXT_LOGO_BLACKPANTHER,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_LIGHT_BLUE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // BLAG
    {
        .names = {"BLAG"},
        .lines = FASTFETCH_DATATEXT_LOGO_BLAG,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // BlankOn
    {
        .names = {"BlankOn"},
        .lines = FASTFETCH_DATATEXT_LOGO_BLANKON,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // BlueLight
    {
        .names = {"BlueLight"},
        .lines = FASTFETCH_DATATEXT_LOGO_BLUELIGHT,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Bodhi
    {
        .names = {"Bodhi"},
        .lines = FASTFETCH_DATATEXT_LOGO_BODHI,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_YELLOW,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Bonsai
    {
        .names = {"Bonsai"},
        .lines = FASTFETCH_DATATEXT_LOGO_BONSAI,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // BSD
    {
        .names = {"BSD"},
        .lines = FASTFETCH_DATATEXT_LOGO_BSD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // BunsenLabs
    {
        .names = {"BunsenLabs"},
        .lines = FASTFETCH_DATATEXT_LOGO_BUNSENLABS,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // LAST
    {},
};

static const FFlogo C[] = {
    // CachyOS
    {
        .names = {"Cachy", "cachyos", "cachy-linux", "cachyos-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CACHYOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // CachyOSSmall
    {
        .names = {"Cachy_small", "cachyos_small", "cachy-linux-small", "cachyos-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_CACHYOS_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Calculate
    {
        .names = {"Calculate"},
        .lines = FASTFETCH_DATATEXT_LOGO_CALCULATE,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // CalinixOS
    {
        .names = {"Calinix", "calinixos"},
        .lines = FASTFETCH_DATATEXT_LOGO_CALINIXOS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // CalinixOSSmall
    {
        .names = {"Calinix_small", "calinixos_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_CALINIXOS_SMALL,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Carbs
    {
        .names = {"Carbs"},
        .lines = FASTFETCH_DATATEXT_LOGO_CARBS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // CBL-Mariner
    {
        .names = {"CBL-Mariner"},
        .lines = FASTFETCH_DATATEXT_LOGO_CBL_MARINER,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // CelOS
    {
        .names = {"Cel", "celos", "cel-linux", "celos-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CELOS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Center
    {
        .names = {"Center"},
        .lines = FASTFETCH_DATATEXT_LOGO_CENTER,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // CentOS
    {
        .names = {"Cent", "centos", "cent-linux", "centos-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CENTOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // CentOSSmall
    {
        .names = {"Cent_small", "centos_small", "cent-linux_small", "cent-linux-small", "centos-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_CENTOS_SMALL,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // Chakra
    {
        .names = {"Chakra"},
        .lines = FASTFETCH_DATATEXT_LOGO_CHAKRA,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // ChaletOS
    {
        .names = {"ChaletOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_CHALETOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Chapeau
    {
        .names = {"Chapeau"},
        .lines = FASTFETCH_DATATEXT_LOGO_CHAPEAU,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Chimera Linux
    {
        .names = {"Chimera Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CHIMERA_LINUX,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // ChonkySealOS
    {
        .names = {"ChonkySealOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_CHONKYSEALOS,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Chrom
    {
        .names = {"Chrom", "ChromeOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_CHROM,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Cleanjaro
    {
        .names = {"Cleanjaro"},
        .lines = FASTFETCH_DATATEXT_LOGO_CLEANJARO,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // CleanjaroSmall
    {
        .names = {"Cleanjaro_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_CLEANJARO_SMALL,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // ClearLinux
    {
        .names = {"Clear Linux", "clearlinux", "Clear Linux OS"},
        .lines = FASTFETCH_DATATEXT_LOGO_CLEAR_LINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // ClearOS
    {
        .names = {"ClearOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_CLEAROS,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Clover
    {
        .names = {"Clover"},
        .lines = FASTFETCH_DATATEXT_LOGO_CLOVER,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Cobalt
    {
        .names = {"Cobalt"},
        .lines = FASTFETCH_DATATEXT_LOGO_COBALT,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_LIGHT_BLUE,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Condres
    {
        .names = {"Condres"},
        .lines = FASTFETCH_DATATEXT_LOGO_CONDRES,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_CYAN
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // ContainerLinux
    {
        .names = {"ContainerLinux", "Container Linux", "Container Linux by CoreOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_COREOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // CRUX
    {
        .names = {"CRUX"},
        .lines = FASTFETCH_DATATEXT_LOGO_CRUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // CRUXSmall
    {
        .names = {"CRUX_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_CRUX_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // CrystalLinux
    {
        .names = {"Crystal", "Crystal", "crystal-linux", "Crystal-Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CRYSTAL,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // Cucumber
    {
        .names = {"Cucumber", "CucumberOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_CUCUMBER,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // CutefishOS
    {
        .names = {"CutefishOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_CUTEFISHOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
    },
    // CuteOS
    {
        .names = {"CuteOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_CUTEOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_256 "57",
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // CyberOS
    {
        .names = {"CyberOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_CYBEROS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_256 "57",
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // cycledream
    {
        .names = {"cycledream"},
        .lines = FASTFETCH_DATATEXT_LOGO_CYCLEDREAM,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // LAST
    {},
};

static const FFlogo D[] = {
    // DahliaOS
    {
        .names = {"dahliaOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_DAHLIA,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // DarkOS
    {
        .names = {"DarkOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_DARKOS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Debian
    {
        .names = {"Debian", "debian-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_DEBIAN,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // DebianSmall
    {
        .names = {"Debian_small", "debian-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_DEBIAN_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Deepin
    {
        .names = {"Deepin", "deepin-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_DEEPIN,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // DesaOS
    {
        .names = {"DesaOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_DESAOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Devuan
    {
        .names = {"Devuan", "devuan-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_DEVUAN,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // DevuanSmall
    {
        .names = {"Devuan_small", "devuan-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_DEVUAN_SMALL,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // DietPi
    {
        .names = {"DietPi"},
        .lines = FASTFETCH_DATATEXT_LOGO_DIETPI,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // DracOS
    {
        .names = {"DracOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_DRACOS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // DragonFly
    {
        .names = {"DragonFly", "DragonFly-BSD"},
        .lines = FASTFETCH_DATATEXT_LOGO_DRAGONFLY,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // DragonFlySmall
    {
        .names = {"DragonFly_small", "DragonFly-BSD_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_DRAGONFLY_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // DragonFlyOld
    {
        .names = {"DragonFly_old", "DragonFly-BSD_old"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_DRAGONFLY_OLD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // DraugerOS
    {
        .names = {"DraugerOS", "Drauger"},
        .lines = FASTFETCH_DATATEXT_LOGO_DRAUGER,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Droidian
    {
        .names = {"Droidian"},
        .lines = FASTFETCH_DATATEXT_LOGO_DROIDIAN,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_LIGHT_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_LIGHT_GREEN,
    },
    // LAST
    {},
};

static const FFlogo E[] = {
    // Elbrus
    {
        .names = {"elbrus"},
        .lines = FASTFETCH_DATATEXT_LOGO_ELBRUS,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Elementary
    {
        .names = {"Elementary"},
        .lines = FASTFETCH_DATATEXT_LOGO_ELEMENTARY,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // ElementarySmall
    {
        .names = {"Elementary_small", "elementary-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ELEMENTARY_SMALL,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Elive
    {
        .names = {"Elive"},
        .lines = FASTFETCH_DATATEXT_LOGO_ELIVE,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_CYAN,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // EncryptOS
    {
        .names = {"EncryptOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_ENCRYPTOS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Endeavour
    {
        .names = {"Endeavour", "endeavour-linux", "endeavouros", "endeavouros-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ENDEAVOUR,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // EndeavourSmall
    {
        .names = {"Endeavour_small", "endeavour-linux_small", "endeavouros_small", "endeavouros-linux_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ENDEAVOUR_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLUE,
        },
    },
    // Endless
    {
        .names = {"Endless"},
        .lines = FASTFETCH_DATATEXT_LOGO_ENDLESS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Enso
    {
        .names = {"Enso"},
        .lines = FASTFETCH_DATATEXT_LOGO_ENSO,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // EshanizedOS
    {
        .names = {"EshanizedOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_ESHANIZEDOS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // EuroLinux
    {
        .names = {"EuroLinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_EUROLINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // evolinx
    {
        .names = {"Evolinx"},
        .lines = FASTFETCH_DATATEXT_LOGO_EVOLINX,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // EvolutionOS
    {
        .names = {"EvolutionOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_EVOLUTIONOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    // EvolutionOSSmall
    {
        .names = {"EvolutionOS_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_EVOLUTIONOS_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    // EvolutionOS_old
    {
        .names = {"EvolutionOS_old"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_EVOLUTIONOS_OLD,
        .colors = {
            FF_COLOR_FG_LIGHT_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Exherbo
    {
        .names = {"Exherbo", "exherbo-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_EXHERBO,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // ExodiaPredator
    {
        .names = {"Exodia Predator", "exodia-predator", "Exodia Predator OS"},
        .lines = FASTFETCH_DATATEXT_LOGO_EXODIA_PREDATOR,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // LAST
    {},
};

static const FFlogo F[] = {
    // Fedora
    {
        .names = {"Fedora", "fedora-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // FedoraAsahiRemix
    {
        .names = {"fedora-asahi-remix"},
        .lines = FASTFETCH_DATATEXT_LOGO_ASAHI,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
            "38", //cyan
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // FedoraSmall
    {
        .names = {"Fedora_small", "fedora-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // FedoraOld
    {
        .names = {"Fedora_old", "fedora-old", "fedora-linux-old", "fedora-linux_old"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_OLD,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // FedoraSilverblue
    {
        .names = {"Fedora_silverblue", "fedora-silverblue", "fedora-linux-silverblue", "fedora-linux_silverblue"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_SILVERBLUE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // FedoraKinoite
    {
        .names = {"Fedora_kinoite", "fedora-kinoite", "fedora-linux-kinoite", "fedora-linux_kinoite"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_KINOITE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // FedoraSericea
    {
        .names = {"Fedora_sericea", "fedora-sericea", "fedora-linux-sericea", "fedora-linux_sericea"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_SERICEA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // FedoraOnyx placeholder - Currently does not have a logo
    //{
        //.names = {"Fedora_onyx", "fedora-onyx", "fedora-linux-onyx", "fedora-linux_onyx"},
        //.type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        //.lines = FASTFETCH_DATATEXT_LOGO_FEDORA_ONYX,
        //.colors = {
            //FF_COLOR_FG_BLUE,
            //FF_COLOR_FG_WHITE,
        //},
        //.colorKeys = FF_COLOR_FG_BLUE,
        //.colorTitle = FF_COLOR_FG_BLUE,
    //},
    // FedoraCoreOS
    {
        .names = {"Fedora_coreos", "fedora-coreos", "fedora-linux-coreos", "fedora-linux_coreos"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_COREOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // FemboyOS
    {
        .names = {"FemboyOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEMBOYOS,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Feren
    {
        .names = {"Feren"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEREN,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Finnix
    {
        .names = {"Finnix"},
        .lines = FASTFETCH_DATATEXT_LOGO_FINNIX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Floflis
    {
        .names = {"Floflis"},
        .lines = FASTFETCH_DATATEXT_LOGO_FLOFLIS,
        .colors = {
            FF_COLOR_FG_LIGHT_CYAN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // FreeBSD
    {
        .names = {"Freebsd"},
        .lines = FASTFETCH_DATATEXT_LOGO_FREEBSD,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // FreeBSDSmall
    {
        .names = {"freebsd_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_FREEBSD_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // FreeMiNT
    {
        .names = {"FreeMiNT"},
        .lines = FASTFETCH_DATATEXT_LOGO_FREEMINT,
        .colors = {
            FF_COLOR_FG_WHITE
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Frugalware
    {
        .names = {"Frugalware", "frugalware-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_FRUGALWARE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Funtoo
    {
        .names = {"Funtoo"},
        .lines = FASTFETCH_DATATEXT_LOGO_FUNTOO,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // LAST
    {},
};

static const FFlogo G[] = {
    // GalliumOS
    {
        .names = {"GalliumOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_GALLIUMOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Garuda
    {
        .names = {"Garuda", "garuda-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_GARUDA,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // GarudaDragon
    {
        .names = {"GarudaDragon", "garuda-dragon", "garuda-linux-dragon"},
        .lines = FASTFETCH_DATATEXT_LOGO_GARUDA_DRAGON,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // GarudaSmall
    {
        .names = {"Garuda_small", "garudalinux_small", "garuda-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_GARUDA_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Gentoo
    {
        .names = {"Gentoo", "gentoo-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_GENTOO,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // GentooSmall
    {
        .names = {"Gentoo_small", "gentoo-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_GENTOO_SMALL,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // GhostBSD
    {
        .names = {"GhostBSD"},
        .lines = FASTFETCH_DATATEXT_LOGO_GHOSTBSD,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Glaucus
    {
        .names = {"Glaucus"},
        .lines = FASTFETCH_DATATEXT_LOGO_GLAUCUS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // GNewSense
    {
        .names = {"gNewSense"},
        .lines = FASTFETCH_DATATEXT_LOGO_GNEWSENSE,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Gnome
    {
        .names = {"Gnome"},
        .lines = FASTFETCH_DATATEXT_LOGO_GNOME,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // GNU
    {
        .names = {"GNU"},
        .lines = FASTFETCH_DATATEXT_LOGO_GNU,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // GoboLinux
    {
        .names = {"GoboLinux", "Gobo"},
        .lines = FASTFETCH_DATATEXT_LOGO_GOBOLINUX,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // GrapheneOS
    {
        .names = {"GrapheneOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_GRAPHENEOS,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Grombyang
    {
        .names = {"Grombyang"},
        .lines = FASTFETCH_DATATEXT_LOGO_GROMBYANG,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Guix
    {
        .names = {"Guix"},
        .lines = FASTFETCH_DATATEXT_LOGO_GUIX,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // GuixSmall
    {
        .names = {"Guix_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_GUIX_SMALL,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // LAST
    {},
};

static const FFlogo H[] = {
    // Haiku
    {
        .names = {"Haiku"},
        .lines = FASTFETCH_DATATEXT_LOGO_HAIKU,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // HaikuSmall
    {
        .names = {"Haiku-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_HAIKU_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // HamoniKR
    {
        .names = {"HamoniKR"},
        .lines = FASTFETCH_DATATEXT_LOGO_HAMONIKR,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_256 "99"
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // HarDClanZ
    {
        .names = {"HarDClanZ"},
        .lines = FASTFETCH_DATATEXT_LOGO_HARDCLANZ,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // HardenedBSD
    {
        .names = {"HardenedBSD"},
        .lines = FASTFETCH_DATATEXT_LOGO_FREEBSD,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Hash
    {
        .names = {"Hash"},
        .lines = FASTFETCH_DATATEXT_LOGO_HASH,
        .colors = {
            FF_COLOR_FG_256 "123",
            FF_COLOR_FG_256 "123",
        },
    },
    // Huayra
    {
        .names = {"Huayra"},
        .lines = FASTFETCH_DATATEXT_LOGO_HUAYRA,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Hybrid
    {
        .names = {"Hybrid"},
        .lines = FASTFETCH_DATATEXT_LOGO_HYBRID,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_LIGHT_BLUE,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // HydroOS
    {
        .names = {"HydroOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_HYDROOS,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Hyperbola
    {
        .names = {"Hyperbola"},
        .lines = FASTFETCH_DATATEXT_LOGO_HYPERBOLA,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // HyperbolaSmall
    {
        .names = {"Hyperbola_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_HYPERBOLA_SMALL,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // LAST
    {},
};

static const FFlogo I[] = {
    // Iglunix
    {
        .names = {"Iglunix", "Iglu"},
        .lines = FASTFETCH_DATATEXT_LOGO_IGLUNIX,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // InstantOS
    {
        .names = {"InstantOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_INSTANTOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Interix
    {
        .names = {"Interix"},
        .lines = FASTFETCH_DATATEXT_LOGO_INTERIX,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_YELLOW,
        },
    },
    // IRIX
    {
        .names = {"IRIX"},
        .lines = FASTFETCH_DATATEXT_LOGO_IRIX,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Ironclad
    {
        .names = {"Ironclad"},
        .lines = FASTFETCH_DATATEXT_LOGO_IRONCLAD,
        .colors = {
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // Itc
    {
        .names = {"Itc"},
        .lines = FASTFETCH_DATATEXT_LOGO_ITC,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // LAST
    {},
};

static const FFlogo J[] = {
    // Januslinux
    {
        .names = {"januslinux", "janus"},
        .lines = FASTFETCH_DATATEXT_LOGO_JANUSLINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // LAST
    {},
};

static const FFlogo K[] = {
    // Kaisen
    {
        .names = {"Kaisen"},
        .lines = FASTFETCH_DATATEXT_LOGO_KAISEN,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Kali
    {
        .names = {"Kali", "Kalilinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_KALI,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // KaliSmall
    {
        .names = {"Kali_small", "Kalilinux_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_KALI_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // KaOS
    {
        .names = {"KaOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_KAOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // KernelOS
    {
        .names = {"KernelOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_KERNELOS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_MAGENTA,
        }
    },
    // KDENeon
    {
        .names = {"KDE", "kde-neon"},
        .lines = FASTFETCH_DATATEXT_LOGO_KDE,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Kibojoe
    {
        .names = {"Kibojoe"},
        .lines = FASTFETCH_DATATEXT_LOGO_KIBOJOE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // KISSLinux
    {
        .names = {"KISS", "kiss-linux", "kisslinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_KISS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Kogaion
    {
        .names = {"Kogaion"},
        .lines = FASTFETCH_DATATEXT_LOGO_KOGAION,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Korora
    {
        .names = {"Korora"},
        .lines = FASTFETCH_DATATEXT_LOGO_KORORA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // KrassOS
    {
        .names = {"KrassOS", "Krass"},
        .lines = FASTFETCH_DATATEXT_LOGO_KRASSOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // KSLinux
    {
        .names = {"KSLinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_KSLINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Kubuntu
    {
        .names = {"Kubuntu", "kubuntu-linux", "kde-ubuntu"},
        .lines = FASTFETCH_DATATEXT_LOGO_KUBUNTU,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // LAST
    {},
};

static const FFlogo L[] = {
    // LangitKetujuh
    {
        .names = {"langitketujuh", "l7"},
        .lines = FASTFETCH_DATATEXT_LOGO_LANGITKETUJUH,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Laxeros
    {
        .names = {"Laxeros"},
        .lines = FASTFETCH_DATATEXT_LOGO_LAXEROS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // LEDE
    {
        .names = {"LEDE"},
        .lines = FASTFETCH_DATATEXT_LOGO_LEDE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // LibreELEC
    {
        .names = {"LibreELEC"},
        .lines = FASTFETCH_DATATEXT_LOGO_LIBREELEC,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // Linspire
    {
        .names = {"Linspire", "Lindows"},
        .lines = FASTFETCH_DATATEXT_LOGO_LINSPIRE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Linux
    {
        .names = {"Linux", "linux-generic"},
        .lines = FASTFETCH_DATATEXT_LOGO_LINUX,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // LinuxSmall
    {
        .names = {"Linux_small", "linux-generic_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_LINUX_SMALL,
        .colors = {
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // LinuxLight
    {
        .names = {"LinuxLite", "Linux Lite", "linux_lite"},
        .lines = FASTFETCH_DATATEXT_LOGO_LINUXLITE,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // LinuxLightSmall
    {
        .names = {"LinuxLite_small", "Linux Lite_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_LINUXLITE_SMALL,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // LinuxMint
    {
        .names = {"linuxmint", "linux-mint"},
        .lines = FASTFETCH_DATATEXT_LOGO_MINT,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // LinuxMintSmall
    {
        .names = {"linuxmint_small", "linux-mint_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MINT_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // LinuxMintOld
    {
        .names = {"linux-mint_old", "linux-mint-old"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MINT_OLD,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Live_Raizo
    {
        .names = {"Live Raizo", "Live_Raizo"},
        .lines = FASTFETCH_DATATEXT_LOGO_LIVE_RAIZO,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // LMDE
    {
        .names = {"LMDE"},
        .lines = FASTFETCH_DATATEXT_LOGO_LMDE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // LainOS
    {
        .names = {"LainOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_LAINOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_256 "14",
            FF_COLOR_FG_WHITE,
        },
    },
    // Lunar
    {
        .names = {"Lunar"},
        .lines = FASTFETCH_DATATEXT_LOGO_LUNAR,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // LAST
    {},
};

static const FFlogo M[] = {
    // MacOS
    {
        .names = {"macos", "mac"},
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS,
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
    // MacOSSmall
    {
        .names = {"macos_small", "mac_small"},
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
    // MacOS2
    {
        .names = {"macos2", "mac2"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS2,
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
    // MacOS2Small
    {
        .names = {"macos2_small", "mac2_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS2_SMALL,
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
    // MainsailOS
    {
        .names = {"MainsailOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_MAINSAILOS,
        .colors = {
            FF_COLOR_FG_RED,
        },
    },
    // MainsailOSSmall
    {
        .names = {"MainsailOS_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MAINSAILOS_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
    },
    // Mageia
    {
        .names = {"Mageia"},
        .lines = FASTFETCH_DATATEXT_LOGO_MAGEIA,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // MageiaSmall
    {
        .names = {"Mageia_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MAGEIA_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // MagpieOS
    {
        .names = {"MagpieOS", "Magpie"},
        .lines = FASTFETCH_DATATEXT_LOGO_MAGPIEOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Mandriva
    {
        .names = {"mandriva", "mandrake"},
        .lines = FASTFETCH_DATATEXT_LOGO_MANDRIVA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // Manjaro
    {
        .names = {"manjaro", "manjaro-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_MANJARO,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // ManjaroSmall
    {
        .names = {"manjaro_small", "manjaro-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MANJARO_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // MassOS
    {
        .names = {"MassOS", "mass"},
        .lines = FASTFETCH_DATATEXT_LOGO_MASSOS,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // MatuusOS
    {
        .names = {"MatuusOS", "Matuus"},
        .lines = FASTFETCH_DATATEXT_LOGO_MATUUSOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // MaUI
    {
        .names = {"MaUI"},
        .lines = FASTFETCH_DATATEXT_LOGO_MAUI,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Meowix
    {
        .names = {"Meowix"},
        .lines = FASTFETCH_DATATEXT_LOGO_MEOWIX,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // Mer
    {
        .names = {"Mer"},
        .lines = FASTFETCH_DATATEXT_LOGO_MER,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Mint
    {
        .names = {"mint", "mint-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_MINT,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // MintSmall
    {
        .names = {"mint_small", "mint-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MINT_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // MintOld
    {
        .names = {"mint_old", "mint-old", "mint-linux_old", "mint-linux-old"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MINT_OLD,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Minix
    {
        .names = {"Minix"},
        .lines = FASTFETCH_DATATEXT_LOGO_MINIX,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // MiracleLinux
    {
        .names = {"MIRACLE LINUX", "miracle_linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_MIRACLE_LINUX,
        .colors = {
            FF_COLOR_FG_256 "29",
        },
        .colorKeys = FF_COLOR_FG_256 "29",
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // MOS
    {
        .names = {"MOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_MOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
        },
    },
    // Msys2
    {
        .names = {"Msys2"},
        .lines = FASTFETCH_DATATEXT_LOGO_MSYS2,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // MX
    {
        .names = {"MX", "MX Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_MX,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // MXSmall
    {
        .names = {"MX_small", "mx linux_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MX_SMALL,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // MX2
    {
        .names = {"MX2"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MX2,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // LAST
    {},
};

static const FFlogo N[] = {
    // Namib
    {
        .names = {"Namib"},
        .lines = FASTFETCH_DATATEXT_LOGO_NAMIB,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // Nekos
    {
        .names = {"Nekos"},
        .lines = FASTFETCH_DATATEXT_LOGO_NEKOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
    },
    // Neptune
    {
        .names = {"Neptune"},
        .lines = FASTFETCH_DATATEXT_LOGO_NEPTUNE,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_WHITE,
        },
    },
    // NetRunner
    {
        .names = {"NetRunner"},
        .lines = FASTFETCH_DATATEXT_LOGO_NETRUNNER,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Nitrux
    {
        .names = {"Nitrux"},
        .lines = FASTFETCH_DATATEXT_LOGO_NITRUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // NixOS
    {
        .names = {"NixOS", "nix", "nixos-linux", "nix-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // NixOSSmall
    {
        .names = {"NixOS_small", "nix_small", "nixos-linux-small", "nix-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // NixOSOld
    {
        .names = {"nixos_old", "nix-old", "nixos-old", "nix_old"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS_OLD,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // NixOsOldSmall
    {
        .names = {"nixos_old_small", "nix-old-small", "nixos-old-small", "nix_old_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS_OLD_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // NetBSD
    {
        .names = {"netbsd"},
        .lines = FASTFETCH_DATATEXT_LOGO_NETBSD,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Nobara
    {
        .names = {"nobara", "nobara-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_NOBARA,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // NomadBSD
    {
        .names = {"nomadbsd"},
        .lines = FASTFETCH_DATATEXT_LOGO_NOMADBSD,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Nurunner
    {
        .names = {"Nurunner"},
        .lines = FASTFETCH_DATATEXT_LOGO_NURUNNER,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // NuTyX
    {
        .names = {"NuTyX"},
        .lines = FASTFETCH_DATATEXT_LOGO_NUTYX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_RED,
        },
    },
    // LAST
    {},
};

static const FFlogo O[] = {
    // Obarun
    {
        .names = {"Obarun"},
        .lines = FASTFETCH_DATATEXT_LOGO_OBARUN,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // OBRevenge
    {
        .names = {"OBRevenge"},
        .lines = FASTFETCH_DATATEXT_LOGO_OBREVENGE,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // OmniOS
    {
        .names = {"OmniOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_OMNIOS,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_LIGHT_BLACK,
        }
    },
    // OpenKylin
    {
        .names = {"openkylin", "open-kylin"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENKYLIN,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // OpenBSD
    {
        .names = {"openbsd"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENBSD,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // OpenBSDSmall
    {
        .names = {"openbsd_small", "openbsd-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_OPENBSD_SMALL,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // OpenEuler
    {
        .names = {"OpenEuler"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENEULER,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // OpenIndiana
    {
        .names = {"OpenIndiana"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENINDIANA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // OpenMamba
    {
        .names = {"OpenMamba"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENMAMBA,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_GREEN,
        },
    },
    // OpenStage
    {
        .names = {"OpenStage"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSTAGE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    // OpenSuse
    {
        .names = {"opensuse", "open_suse", "open-suse"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // OpenSuseSmall
    {
        .names = {"opensuse_small", "open_suse_small", "open-suse_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // openSuseMicroOS
    {
        .names = {"opensuse-microos", "opensuse_microos"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_MICROOS,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
    },
    // OpenSuseLeap
    {
        .names = {"opensuse_leap", "open_suse_leap", "opensuse-leap", "open-suse-leap", "suse_leap", "suse-leap", "opensuseleap"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_LEAP,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // OpenSuseTumbleweed
    {
        .names = {"opensuse_tumbleweed", "open_suse_tumbleweed", "opensuse-tumbleweed", "open-suse-tumbleweed", "suse_tumbleweed", "suse-tumbleweed", "opensusetumbleweed"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // openSUSESlowroll
    {
        .names = {"opensuse_slowroll", "opensuse-slowroll", "opensuse-tumbleweed-slowroll"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_SLOWROLL,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // OpenMandriva
    {
        .names = {"openmandriva", "open-mandriva", "open_mandriva", "openmandriva lx"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENMANDRIVA,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // OpenWrt
    {
        .names = {"openwrt"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENWRT,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // OPNsense
    {
        .names = {"OPNsense"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPNSENSE,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_256 "202",
        },
    },
    // Oracle
    {
        .names = {"oracle", "oracle linux", "oracle linux server"},
        .lines = FASTFETCH_DATATEXT_LOGO_ORACLE,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Orchid
    {
        .names = {"orchid"},
        .lines = FASTFETCH_DATATEXT_LOGO_ORCHID,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // OrchidSmall
    {
        .names = {"orchid_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ORCHID_SMALL,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // OS_Elbrus
    {
        .names = {"OS Elbrus"},
        .lines = FASTFETCH_DATATEXT_LOGO_OS_ELBRUS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // OSMC
    {
        .names = {"OSMC", "Open Source Media Center"},
        .lines = FASTFETCH_DATATEXT_LOGO_OSMC,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // OSX
    {
        .names = {"OSX"},
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS,
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
    // OSXSmall
    {
        .names = {"OSX_small"},
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
    // LAST
    {},
};

static const FFlogo P[] = {
    // PacBSD
    {
        .names = {"PacBSD"},
        .lines = FASTFETCH_DATATEXT_LOGO_PACBSD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // Panwah
    {
        .names = {"Panwah"},
        .lines = FASTFETCH_DATATEXT_LOGO_PANWAH,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_BLACK,
        },
    },
    // Parabola
    {
        .names = {"parabola", "parabola-gnulinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_PARABOLA,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // ParabolaSmall
    {
        .names = {"parabola_small", "parabola-gnulinux_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_PARABOLA_SMALL,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // Parch
    {
        .names = {"Parch"},
        .lines = FASTFETCH_DATATEXT_LOGO_PARCH,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
    },
    // Pardus
    {
        .names = {"Pardus"},
        .lines = FASTFETCH_DATATEXT_LOGO_PARDUS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // Parrot
    {
        .names = {"Parrot"},
        .lines = FASTFETCH_DATATEXT_LOGO_PARROT,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // Parsix
    {
        .names = {"Parsix"},
        .lines = FASTFETCH_DATATEXT_LOGO_PARSIX,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
    },
    // PCBSD
    {
        .names = {"PCBSD", "TrueOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_PCBSD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // PCLinuxOS
    {
        .names = {"PCLinuxOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_PCLINUXOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // PearOS
    {
        .names = {"PearOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_PEAROS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLUE,
        },
    },
    // Pengwin
    {
        .names = {"Pengwin"},
        .lines = FASTFETCH_DATATEXT_LOGO_PENGWIN,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_LIGHT_MAGENTA,
            FF_COLOR_FG_MAGENTA,
        },
    },
    // Pentoo
    {
        .names = {"Pentoo"},
        .lines = FASTFETCH_DATATEXT_LOGO_PENTOO,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
    },
    // Peppermint
    {
        .names = {"Peppermint"},
        .lines = FASTFETCH_DATATEXT_LOGO_PEPPERMINT,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // Peropesis
    {
        .names = {"Peropesis", "Peropesis Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_PEROPESIS,
        .colors = {
            FF_COLOR_FG_WHITE
        },
    },
    // PhyOS
    {
        .names = {"PhyOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_PHYOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // PikaOS
    {
        .names = {"PikaOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_PIKAOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
    },
    // Pisi
    {
        .names = {"Pisi"},
        .lines = FASTFETCH_DATATEXT_LOGO_PISI,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // PNMLinux
    {
        .names = {"PNM Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_PNM_LINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_256 "202"
        },
    },
    // Pop
    {
        .names = {"pop", "popos", "pop_os", "pop-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_POP,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // PopSmall
    {
        .names = {"pop_small", "popos_small", "pop_os_small", "pop-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_POP_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Porteus
    {
        .names = {"Porteus"},
        .lines = FASTFETCH_DATATEXT_LOGO_PORTEUS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // PostMarketOS
    {
        .names = {"PostMarketOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_POSTMARKETOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    // PostMarketOSSmall
    {
        .names = {"PostMarketOS_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_POSTMARKETOS_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    // Proxmox
    {
        .names = {"Proxmox", "pve"},
        .lines = FASTFETCH_DATATEXT_LOGO_PROXMOX,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_256 "202"
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_256 "202",
    },
    // PuffOS
    {
        .names = {"PuffOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_PUFFOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    // Puppy
    {
        .names = {"Puppy"},
        .lines = FASTFETCH_DATATEXT_LOGO_PUPPY,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // PureOS
    {
        .names = {"PureOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_PUREOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    // PureOSSmall
    {
        .names = {"PureOS_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_PUREOS_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    // LAST
    {},
};

static const FFlogo Q[] = {
    // Q4OS
    {
        .names = {"Q4OS"},
        .lines = FASTFETCH_DATATEXT_LOGO_Q4OS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_RED,
        },
    },
    // Qubes
    {
        .names = {"Qubes"},
        .lines = FASTFETCH_DATATEXT_LOGO_QUBES,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
        },
    },
    // Qubyt
    {
        .names = {"Qubyt"},
        .lines = FASTFETCH_DATATEXT_LOGO_QUBYT,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLACK,
        },
    },
    // Quibian
    {
        .names = {"Quibian"},
        .lines = FASTFETCH_DATATEXT_LOGO_QUIBIAN,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    // LAST
    {},
};

static const FFlogo R[] = {
    // Radix
    {
        .names = {"Radix"},
        .lines = FASTFETCH_DATATEXT_LOGO_RADIX,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
        },
    },
    // Raspbian
    {
        .names = {"raspbian", "raspi", "raspberrypi", "raspberrypios"},
        .lines = FASTFETCH_DATATEXT_LOGO_RASPBIAN,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // RaspbianSmall
    {
        .names = {"raspbian_small", "raspi_small", "raspberrypi_small", "raspberrypios_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_RASPBIAN_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // RavynOS
    {
        .names = {"RavynOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_RAVYNOS,
        .colors = {
            FF_COLOR_FG_256 "15",
            FF_COLOR_FG_WHITE,
        },
    },
    // Reborn
    {
        .names = {"Reborn", "Reborn OS", "reborn-os", "rebornos", "rebornos-linux", "reborn-os-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_REBORN,
        .colors = {
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // RebornSmall
    {
        .names = {"Reborn_small", "Reborn OS_small", "reborn-os-small", "rebornos_small", "rebornos-linux-small", "reborn-os-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_REBORN_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // RedCore
    {
        .names = {"RedCore"},
        .lines = FASTFETCH_DATATEXT_LOGO_REDCORE,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // RedHatEnterpriseLinux
    {
        .names = {"rhel", "redhat", "redhat-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_RHEL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // RedHatEnterpriseLinux_old
    {
        .names = {"rhel_old", "redhat_old", "redhat-linux_old"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_RHEL_OLD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // RedstarOS
    {
        .names = {"redstar", "redstar-os", "redstaros", "redstaros-linux", "redstar-os-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_REDSTAR,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Refracted Devuan
    {
        .names = {"Refracted Devuan", "refracted-devuan"},
        .lines = FASTFETCH_DATATEXT_LOGO_REFRACTED_DEVUAN,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Regata
    {
        .names = {"Regata"},
        .lines = FASTFETCH_DATATEXT_LOGO_REGATA,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
        },
    },
    // Regolith
    {
        .names = {"Regolith"},
        .lines = FASTFETCH_DATATEXT_LOGO_REGOLITH,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // RhaymOS
    {
        .names = {"RhaymOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_RHAYMOS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // RockyLinux
    {
        .names = {"rocky", "rocky-linux", "rockylinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ROCKY,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // RockyLinuxSmall
    {
        .names = {"rocky_small", "rocky-linux_small", "rockylinux_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ROCKY_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // RosaLinux
    {
        .names = {"rosa", "rosa-linux", "rosalinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ROSA,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // LAST
    {},
};

static const FFlogo S[] = {
    // Sabayon
    {
        .names = {"Sabayon"},
        .lines = FASTFETCH_DATATEXT_LOGO_SABAYON,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Sabotage
    {
        .names = {"Sabotage"},
        .lines = FASTFETCH_DATATEXT_LOGO_SABOTAGE,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Sailfish
    {
        .names = {"Sailfish"},
        .lines = FASTFETCH_DATATEXT_LOGO_SAILFISH,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
        },
    },
    // SalentOS
    {
        .names = {"SalentOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_SALENTOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    // SalientOS
    {
        .names = {"Salient OS", "SalientOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_SALIENTOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // Salix
    {
        .names = {"Salix"},
        .lines = FASTFETCH_DATATEXT_LOGO_SALIX,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_GREEN,
        },
    },
    // SambaBOX
    {
        .names = {"SambaBOX", "Profelis SambaBOX"},
        .lines = FASTFETCH_DATATEXT_LOGO_SAMBABOX,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_CYAN,
        },
    },
    // Sasanqua
    {
        .names = {"Sasanqua"},
        .lines = FASTFETCH_DATATEXT_LOGO_SASANQUA,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_RED,
        },
    },
    // Scientific
    {
        .names = {"Scientific"},
        .lines = FASTFETCH_DATATEXT_LOGO_SCIENTIFIC,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
    },
    // Semc
    {
        .names = {"semc"},
        .lines = FASTFETCH_DATATEXT_LOGO_SEMC,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_RED,
        },
    },
    // Septor
    {
        .names = {"Septor"},
        .lines = FASTFETCH_DATATEXT_LOGO_SEPTOR,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
    },
    // Serene
    {
        .names = {"Serene"},
        .lines = FASTFETCH_DATATEXT_LOGO_SERENE,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    // SharkLinux
    {
        .names = {"SharkLinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_SHARKLINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // ShastraOS
    {
        .names = {"ShastraOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_SHASTRAOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // Siduction
    {
        .names = {"Siduction"},
        .lines = FASTFETCH_DATATEXT_LOGO_SIDUCTION,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        }
    },
    // SkiffOS
    {
        .names = {"SkiffOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_SKIFFOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Slitaz
    {
        .names = {"Slitaz"},
        .lines = FASTFETCH_DATATEXT_LOGO_SLITAZ,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_YELLOW,
        },
    },
    // Slackel
    {
        .names = {"Slackel"},
        .lines = FASTFETCH_DATATEXT_LOGO_SLACKEL,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_YELLOW,
        },
    },
    // Slackware
    {
        .names = {"slackware", "slackware-linux", "slackwarelinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_SLACKWARE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // SlackwareSmall
    {
        .names = {"slackware-small", "slackware-linux-small", "slackware_small", "slackwarelinux_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_SLACKWARE_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // SmartOS
    {
        .names = {"SmartOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_SMARTOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // Soda
    {
        .names = {"Soda"},
        .lines = FASTFETCH_DATATEXT_LOGO_SODA,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // Source Mage
    {
        .names = {"Source Mage", "Source Mage GNU/Linux", "source_mage", "sourcemage"},
        .lines = FASTFETCH_DATATEXT_LOGO_SOURCE_MAGE,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Solaris
    {
        .names = {"solaris", "sunos"},
        .lines = FASTFETCH_DATATEXT_LOGO_SOLARIS,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // SolarisSmall
    {
        .names = {"solaris-small", "solaris_small", "sunos-small", "sunos_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_SOLARIS_SMALL,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Solus
    {
        .names = {"solus", "solus-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_SOLUS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Sparky
    {
        .names = {"Sparky"},
        .lines = FASTFETCH_DATATEXT_LOGO_SPARKY,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE
        },
    },
    // Star
    {
        .names = {"Star"},
        .lines = FASTFETCH_DATATEXT_LOGO_STAR,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_WHITE,
        },
    },
    // StockLinux
    {
        .names = {"Stock Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_STOCK_LINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // SteamOS
    {
        .names = {"steamos"},
        .lines = FASTFETCH_DATATEXT_LOGO_STEAMOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Sulin
    {
        .names = {"Sulin"},
        .lines = FASTFETCH_DATATEXT_LOGO_SULIN,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Suse
    {
        .names = {"suse", "suse-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_SUSE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_GREEN,
        },
    },
    // SuseSmall
    {
        .names = {"suse_small", "suse-linux_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_GREEN,
        },
    },
    // Swagarch
    {
        .names = {"Swagarch"},
        .lines = FASTFETCH_DATATEXT_LOGO_SWAGARCH,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // LAST
    {},
};

static const FFlogo T[] = {
    // T2
    {
        .names = {"T2"},
        .lines = FASTFETCH_DATATEXT_LOGO_T2,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
    },
    // Tails
    {
        .names = {"Tails"},
        .lines = FASTFETCH_DATATEXT_LOGO_TAILS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Tatra
    {
        .names = {"Tatra"},
        .lines = FASTFETCH_DATATEXT_LOGO_TATRA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_GREEN,
        },
    },
    // TeArch
    {
        .names = {"TeArch"},
        .lines = FASTFETCH_DATATEXT_LOGO_TEARCH,
        .colors = {
            FF_COLOR_FG_256 "39",
            FF_COLOR_FG_WHITE,
        },
    },
    // TileOS
    {
        .names = {"TileOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_TILEOS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_GREEN,
        },
    },
    // TorizonCore
    {
        .names = {"TorizonCore"},
        .lines = FASTFETCH_DATATEXT_LOGO_TORIZONCORE,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_MAGENTA,
        },
    },
    // Trisquel
    {
        .names = {"Trisquel"},
        .lines = FASTFETCH_DATATEXT_LOGO_TRISQUEL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // TuxedoOS
    {
        .names = {"Tuxedo OS"},
        .lines = FASTFETCH_DATATEXT_LOGO_TUXEDO_OS,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
    },
    // Twister
    {
        .names = {"Twister"},
        .lines = FASTFETCH_DATATEXT_LOGO_TWISTER,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
    },
    // LAST
    {},
};

static const FFlogo U[] = {
    // Ubuntu
    {
        .names = {"ubuntu", "ubuntu-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // UbuntuBudgie
    {
        .names = {"ubuntu budgie", "ubuntu-budgie"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_BUDGIE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // UbuntuCinnamon
    {
        .names = {"ubuntu cinnamon", "ubuntu-cinnamon"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_CINNAMON,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // UbuntuGnome
    {
        .names = {"ubuntu gnome", "ubuntu-gnome"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_GNOME,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // UbuntuKylin
    {
        .names = {"ubuntu kylin", "ubuntu-kylin"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_KYLIN,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // UbuntuMate
    {
        .names = {"ubuntu mate", "ubuntu-mate"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_MATE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // UbuntuOld
    {
        .names = {"ubuntu_old", "ubuntu-linux_old"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_OLD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // UbuntuKde
    {
        .names = {"ubuntu kde", "ubuntu-kde", "ubuntu-plasma"},
        .lines = FASTFETCH_DATATEXT_LOGO_KUBUNTU,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // UbuntuSmall
    {
        .names = {"ubuntu_small", "ubuntu-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // UbuntuStudio
    {
        .names = {"ubuntu studio", "ubuntu-studio"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_STUDIO,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // UbuntuSway
    {
        .names = {"ubuntu sway", "ubuntu-sway"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_SWAY,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // UbuntuTouch
    {
        .names = {"ubuntu touch", "ubuntu-touch"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_TOUCH,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    // UbuntuUnity
    {
        .names = {"ubuntu unity", "ubuntu-unity"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_UNITY,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
    },
    // Ubuntu2Small
    {
        .names = {"ubuntu2_small", "ubuntu2-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU2_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Ubuntu2Old
    {
        .names = {"ubuntu2_old"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU2_OLD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
    },
    // Ultramarine
    {
        .names = {"Ultramarine", "Ultramarine Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ULTRAMARINE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Univalent
    {
        .names = {"Univalent"},
        .lines = FASTFETCH_DATATEXT_LOGO_UNIVALENT,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    // Univention
    {
        .names = {"Univention"},
        .lines = FASTFETCH_DATATEXT_LOGO_UNIVENTION,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // UOS
    {
        .names = {"UOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_UOS,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // UrukOS
    {
        .names = {"UrukOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_URUKOS,
        .colors = {
            FF_COLOR_FG_LIGHT_BLUE,
            FF_COLOR_FG_LIGHT_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_BLUE,
            FF_COLOR_FG_BLUE,
        }
    },
    // Uwuntu
    {
        .names = {"uwuntu"},
        .lines = FASTFETCH_DATATEXT_LOGO_UWUNTU,
        .colors = {
            FF_COLOR_FG_256 "225",
            FF_COLOR_FG_256 "206",
            FF_COLOR_FG_256 "52",
        },
    },
    // LAST
    {},
};

static const FFlogo V[] = {
    // Vanilla
    {
        .names = {"vanilla", "vanilla-os", "vanilla-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_VANILLA,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // Venom
    {
        .names = {"Venom"},
        .lines = FASTFETCH_DATATEXT_LOGO_VENOM,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_BLUE,
        },
    },
    // VenomSmall
    {
        .names = {"Venom_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_VENOM_SMALL,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_BLUE,
        },
    },
    // Vnux
    {
        .names = {"Vnux"},
        .lines = FASTFETCH_DATATEXT_LOGO_VNUX,
        .colors = {
            FF_COLOR_FG_256 "11",
            FF_COLOR_FG_256 "8",
            FF_COLOR_FG_256 "15",
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // Vzlinux
    {
        .names = {"Vzlinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_VZLINUX,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
    },
    // Void
    {
        .names = {"void", "void-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_VOID,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // VoidSmall
    {
        .names = {"void_small", "void-linux-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_VOID_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // LAST
    {},
};

static const FFlogo W[] = {
    // WiiLinuxNgx
    {
        .names = {"WiiLinuxNgx"},
        .lines = FASTFETCH_DATATEXT_LOGO_WII_LINUX_NGX,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // Windows11
    {
        .names = {"Windows 11", "Windows Server 2022"},
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_11,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Windows11Small
    {
        .names = {"Windows 11_small", "Windows 11-small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_11_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Windows8
    {
        .names = {"Windows 8", "Windows 8.1", "Windows 10", "Windows Server 2012", "Windows Server 2012 R2", "Windows Server 2016", "Windows Server 2019"},
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_8,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Windows
    {
        .names = {"Windows", "Windows 7", "Windows Server 2008", "Windows Server 2008 R2"},
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Windows95
    {
        .names = {"Windows 95", "Windows 9x"},
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_95,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // LAST
    {},
};

static const FFlogo X[] = {
    // Xenia
    {
        .names = {"Xenia"},
        .lines = FASTFETCH_DATATEXT_LOGO_XENIA,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
        }
    },
    // Xferience
    {
        .names = {"Xferience"},
        .lines = FASTFETCH_DATATEXT_LOGO_XFERIENCE,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    //Xray_OS
    {
        .names = {"Xray_OS"},
        .lines = FASTFETCH_DATATEXT_LOGO_XRAY_OS,
        .colors = {
            FF_COLOR_FG_256 "15",
            FF_COLOR_FG_256 "14",
            FF_COLOR_FG_256 "16",
        }
    },
    // LAST
    {},
};

static const FFlogo Y[] = {
    // YiffOS
    {
        .names = {"YiffOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_YIFFOS,
        .colors = {
            FF_COLOR_FG_256 "93",
            FF_COLOR_FG_256 "92",
        },
    },
    // LAST
    {},
};

static const FFlogo Z[] = {
    // Zorin
    {
        .names = {"zorin", "zorin-linux", "zorinos", "zorinos-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ZORIN,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Z/OS
    {
        .names = {"z/OS", "zos"},
        .lines = FASTFETCH_DATATEXT_LOGO_ZOS,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
    },
    // LAST
    {},
};

const FFlogo* ffLogoBuiltins[] = {
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
};
