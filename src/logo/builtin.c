#include "logo.h"
#include "logo_builtin.h"
#include "common/color.h"

const FFlogo ffLogoUnknown = {
    .names = {"unknown"},
    .lines = FASTFETCH_DATATEXT_LOGO_UNKNOWN,
    .colors = {
        FF_COLOR_FG_DEFAULT,
    },
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
    // Aeon
    {
        .names = {"Aeon"},
        .lines = FASTFETCH_DATATEXT_LOGO_AEON,
        .colors = {
            FF_COLOR_FG_256 "36",
            FF_COLOR_FG_256 "36",
        },
    },
    // Aeon
    {
        .names = {"AerynOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_AERYNOS,
        .colors = {
            FF_COLOR_FG_DEFAULT,
            FF_COLOR_FG_MAGENTA,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // AlmaLinux
    {
        .names = {"Almalinux"},
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
        .names = {"Alpine"},
        .lines = FASTFETCH_DATATEXT_LOGO_ALPINE,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Alpine2
    {
        .names = {"Alpine2"},
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
        .names = {"Alpine_small"},
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
        .names = {"alpine2_small"},
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
        .names = {"alpine3_small"},
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
        .names = {"Alter"},
        .lines = FASTFETCH_DATATEXT_LOGO_ALTER,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // ALTLinux
    {
        .names = {"ALTLinux"},
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
        .colorKeys = FF_COLOR_FG_DEFAULT,
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
        .colorKeys = FF_COLOR_FG_DEFAULT,
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
        .names = {"android_small"},
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
        .names = {"anduinos"},
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // AnushOS
    {
        .names = {"AnushOS"},
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
        .names = {"Aosc OS/Retro", "aoscosretro"},
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
        .names = {"Aosc OS/Retro_small", "aoscosretro_small"},
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
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Aperture
    {
        .names = {"Aperture"},
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
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // ArchBox
    {
        .names = {"ArchBox"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARCHBOX,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .names = {"arch", "archmerge"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    // Arch2
    {
        .names = {"arch2"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH2,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    // Arch3
    {
        .names = {"arch3"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH3,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    // ArchSmall
    {
        .names = {"arch_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    // ArchOld
    {
        .names = {"arch_old"},
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
    // Arkane
    {
        .names = {"Arkane", "Arkane Linux"},
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
        .names = {"Armbian"},
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
        .names = {"Armbian2"},
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
        .names = {"artix_small", "artixlinux_small"},
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
        .names = {"artix2_small", "artixlinux2_small"},
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
    // AthenaOS
    {
        .names = {"AthenaOS"},
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
        .names = {"AthenaOS_old"},
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
        .names = {"Aurora"},
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
        .names = {"AxOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_AXOS,
        .colors = {
            FF_COLOR_FG_RGB "222;6;255",
            FF_COLOR_FG_RGB "222;6;255",
        },
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
        .names = {"bedrock"},
        .lines = FASTFETCH_DATATEXT_LOGO_BEDROCK,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK, //grey
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK, //grey
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // BedrockSmall
    {
        .names = {"bedrock_small"},
        .lines = FASTFETCH_DATATEXT_LOGO_BEDROCK_SMALL,
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK, //grey
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK, //grey
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorKeys = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // BlueLight
    {
        .names = {"BlueLight"},
        .lines = FASTFETCH_DATATEXT_LOGO_BLUELIGHT,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
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
    // BredOS
    {
        .names = {"Bredos"},
        .lines = FASTFETCH_DATATEXT_LOGO_BREDOS,
        .colors = {
            FF_COLOR_FG_RGB "198;151;66", //grey
        },
        .colorKeys = FF_COLOR_FG_RGB "198;151;66",
        .colorTitle = FF_COLOR_FG_RGB "198;151;66",
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // BunsenLabs
    {
        .names = {"BunsenLabs"},
        .lines = FASTFETCH_DATATEXT_LOGO_BUNSENLABS,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // LAST
    {},
};

static const FFlogo C[] = {
    // CachyOS
    {
        .names = {"CachyOS"},
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
        .names = {"CachyOS_small"},
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // CentOS
    {
        .names = {"CentOS"},
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
        .names = {"CentOS_small"},
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
    // Cereus
    {
        .names = {"Cereus", "Cereus Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CEREUS,
        .colors = {
            FF_COLOR_FG_256 "173",
            FF_COLOR_FG_256 "108",
            FF_COLOR_FG_256 "71",
            FF_COLOR_FG_256 "151",
            FF_COLOR_FG_256 "72"
        },
        .colorKeys = FF_COLOR_FG_256 "108",
        .colorTitle = FF_COLOR_MODE_BOLD FF_COLOR_FG_WHITE,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Chimera
    {
        .names = {"Chimera"},
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
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // CleanjaroSmall
    {
        .names = {"Cleanjaro_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_CLEANJARO_SMALL,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
    // Codex Linux (reMarkable OS)
    {
        .names = {"Codex Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CODEX,
        .colors = {
            FF_COLOR_FG_WHITE
        },
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Common Torizon
    {
        .names = {"common-torizon"},
        .lines = FASTFETCH_DATATEXT_LOGO_TORIZONCORE,
        .colors = {
            FF_COLOR_FG_LIGHT_WHITE,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE
        },
    },
    // Cosmic DE
    {
        .names = {"Cosmic"},
        .lines = FASTFETCH_DATATEXT_LOGO_COSMIC,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_YELLOW,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_LIGHT_RED,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_RED,
        .colorTitle = FF_COLOR_FG_YELLOW,
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
    // CuerdOS
    {
        .names = {"CuerdOS", "CuerdOS GNU/Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CUERDOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .names = {"Debian_small", "debian-linux_small"},
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Devuan
    {
        .names = {"Devuan"},
        .lines = FASTFETCH_DATATEXT_LOGO_DEVUAN,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // DevuanSmall
    {
        .names = {"Devuan_small"},
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
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // DragonFly
    {
        .names = {"DragonFly"},
        .lines = FASTFETCH_DATATEXT_LOGO_DRAGONFLY,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // DragonFlySmall
    {
        .names = {"DragonFly_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_DRAGONFLY_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // DragonFlyOld
    {
        .names = {"DragonFly_old"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_DRAGONFLY_OLD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_DEFAULT,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // ElementarySmall
    {
        .names = {"Elementary_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ELEMENTARY_SMALL,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // EmperorOS
    {
        .names = {"Emperor"},
        .lines = FASTFETCH_DATATEXT_LOGO_EMPEROROS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_DEFAULT,
        },
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
    // EndeavourOS
    {
        .names = {"EndeavourOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_ENDEAVOUROS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // EndeavourOSSmall
    {
        .names = {"EndeavourOS_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ENDEAVOUROS_SMALL,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Enso
    {
        .names = {"Enso"},
        .lines = FASTFETCH_DATATEXT_LOGO_ENSO,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
    // eweOS
    {
        .names = {"eweOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_EWEOS,
        .colors = {
           FF_COLOR_FG_WHITE,
           FF_COLOR_FG_LIGHT_YELLOW,
           FF_COLOR_FG_LIGHT_RED,
           FF_COLOR_FG_LIGHT_BLACK,
           FF_COLOR_FG_RED,
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
    // Fastfetch
    {
        .names = {"Fastfetch", "FF"},
        .lines = FASTFETCH_DATATEXT_LOGO_FASTFETCH,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_DEFAULT,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // Fedora
    {
        .names = {"Fedora"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // FedoraAsahiRemix
    {
        .names = {"fedora-asahi-remix"},
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
    // FedoraSmall
    {
        .names = {"Fedora_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    {
        .names = {"Fedora2_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA2_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // FedoraOld
    {
        .names = {"Fedora_old"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_OLD,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // FedoraSilverblue
    {
        .names = {"Fedora-Silverblue"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_SILVERBLUE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // FedoraKinoite
    {
        .names = {"Fedora-Kinoite"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_KINOITE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // FedoraSericea
    {
        .names = {"Fedora-Sericea"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_SERICEA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // FedoraCoreOS
    {
        .names = {"Fedora-CoreOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_COREOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // FemboyOS
    {
        .names = {"FemboyOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEMBOYOS,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Feren
    {
        .names = {"Feren"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEREN,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Filotimo
    {
        .names = {"filotimo"},
        .lines = FASTFETCH_DATATEXT_LOGO_FILOTIMO,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Floflis
    {
        .names = {"Floflis"},
        .lines = FASTFETCH_DATATEXT_LOGO_FLOFLIS,
        .colors = {
            FF_COLOR_FG_LIGHT_CYAN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Furreto
    {
        .names = {"Furreto"},
        .lines = FASTFETCH_DATATEXT_LOGO_FURRETO,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .names = {"GarudaDragon", "garuda-dragon"},
        .lines = FASTFETCH_DATATEXT_LOGO_GARUDA_DRAGON,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // GarudaSmall
    {
        .names = {"Garuda_small", "garuda-linux_small"},
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
        .names = {"Gentoo"},
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
        .names = {"Gentoo_small"},
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
    // GhostFreak
    {
        .names = {"GhostFreak"},
        .lines = FASTFETCH_DATATEXT_LOGO_GHOSTFREAK,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_BLUE,
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
    // GNOME
    {
        .names = {"GNOME"},
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
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
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
    // GoldenDogLinux
    {
        .names = {"GoldenDog Linux", "GDL", "goldendoglinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_GOLDENDOGLINUX,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // GrapheneOS
    {
        .names = {"GrapheneOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_GRAPHENEOS,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // GXDE
    {
        .names = {"GXDE"},
        .lines = FASTFETCH_DATATEXT_LOGO_DEEPIN,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
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
            FF_COLOR_FG_DEFAULT,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // Haiku2
    {
        .names = {"Haiku2"},
        .lines = FASTFETCH_DATATEXT_LOGO_HAIKU2,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // HaikuSmall
    {
        .names = {"Haiku_small"},
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
    // HarmonyOS
    {
        .names = {"HarmonyOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_HARMONYOS,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
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
    // HeliumOS
    {
        .names = {"HeliumOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_HELIUMOS,
        .colors = {
            FF_COLOR_FG_256 "81",
        },
        .colorKeys = FF_COLOR_FG_256 "81",
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Huawei Cloud EulerOS
    {
        .names = {"Huawei Cloud EulerOS", "hce"},
        .lines = FASTFETCH_DATATEXT_LOGO_HCE,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Huayra
    {
        .names = {"Huayra"},
        .lines = FASTFETCH_DATATEXT_LOGO_HUAYRA,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
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
    // HyprOS
    {
        .names = {"hypros"},
        .lines = FASTFETCH_DATATEXT_LOGO_HYPROS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
        },
    },
    // Hyperbola
    {
        .names = {"Hyperbola"},
        .lines = FASTFETCH_DATATEXT_LOGO_HYPERBOLA,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // HydraPWK
    {
        .names = {"HydraPWK"},
        .lines = FASTFETCH_DATATEXT_LOGO_HYDRAPWK,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_DEFAULT,
        },
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Ironclad
    {
        .names = {"Ironclad"},
        .lines = FASTFETCH_DATATEXT_LOGO_IRONCLAD,
        .colors = {
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // Itc
    {
        .names = {"Itc"},
        .lines = FASTFETCH_DATATEXT_LOGO_ITC,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Kali
    {
        .names = {"Kali"},
        .lines = FASTFETCH_DATATEXT_LOGO_KALI,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // KaliSmall
    {
        .names = {"Kali_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_KALI_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Kalpa Desktop
    {
        .names = {"kalpa-desktop"},
        .lines = FASTFETCH_DATATEXT_LOGO_KALPA_DESKTOP,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
    // KDELinux
    {
        .names = {"kdelinux", "kde-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_KDELINUX,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE
        }
    },
    // KDE Neon
    {
        .names = {"KDE Neon"}, // Distro ID is "neon"; Distro name is "KDE Neon"
        .lines = FASTFETCH_DATATEXT_LOGO_KDENEON,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_DEFAULT,
        },
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
    // Kylin
    {
        .names = {"Kylin", "kylin"},
        .lines = FASTFETCH_DATATEXT_LOGO_KYLIN,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_BLACK
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // LAST
    {},
};

static const FFlogo L[] = {
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
    // Lilidog
    {
        .names = {"Lilidog"},
        .lines = FASTFETCH_DATATEXT_LOGO_LILIDOG,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Lingmo OS
    {
        .names = {"Lingmo", "LingmoOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_LINGMO,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
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
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // LinuxFromScratch
    {
        .names = {"LinuxFromScratch", "lfs"},
        .lines = FASTFETCH_DATATEXT_LOGO_LFS,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_BLACK,
        .colorTitle = FF_COLOR_FG_YELLOW,
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
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // LinuxLight
    {
        .names = {"LinuxLite", "Linux Lite", "linux_lite"},
        .lines = FASTFETCH_DATATEXT_LOGO_LINUXLITE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // LinuxLightSmall
    {
        .names = {"LinuxLite_small", "Linux Lite_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_LINUXLITE_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // LinuxMint
    {
        .names = {"linuxmint", "linux-mint"},
        .lines = FASTFETCH_DATATEXT_LOGO_LINUXMINT,
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
        .lines = FASTFETCH_DATATEXT_LOGO_LINUXMINT_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // LinuxMintOld
    {
        .names = {"linuxmint_old", "linux-mint_old"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_LINUXMINT_OLD,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // LliureX
    {
        .names = {"LliureX"},
        .lines = FASTFETCH_DATATEXT_LOGO_LLIUREX,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_BLUE,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Loc-OS
    {
        .names = {"locos", "loc-os", "Loc-OS Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_LOCOS,
        .colors = {
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Lubuntu
    {
        .names = {"lubuntu"},
        .lines = FASTFETCH_DATATEXT_LOGO_LUBUNTU,
        .colors = {
            FF_COLOR_FG_BLUE,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
    // MacOS3
    {
        .names = {"macos3", "mac3"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS3,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Magix
    {
        .names = {"Magix","MagixOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_MAGIX,
        .colors = {
            FF_COLOR_FG_LIGHT_MAGENTA,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_LIGHT_MAGENTA,
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
        .names = {"manjaro", "manjaro-arm"},
        .lines = FASTFETCH_DATATEXT_LOGO_MANJARO,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // ManjaroSmall
    {
        .names = {"manjaro_small", "manjaro-arm_small"},
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
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Mauna
    {
        .names = {"Mauna"},
        .lines = FASTFETCH_DATATEXT_LOGO_MAUNA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // MidnightBSD
    {
        .names = {"MidnightBSD"},
        .lines = FASTFETCH_DATATEXT_LOGO_MIDNIGHTBSD,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // MidOS
    {
        .names = {"MidOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_MIDOS,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // MidOSOld
    {
        .names = {"MidOS_old"},
        .lines = FASTFETCH_DATATEXT_LOGO_MIDOS_OLD,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Minimal System
    {
        .names = {"Minimal_System"},
        .lines = FASTFETCH_DATATEXT_LOGO_MINIMAL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
            FF_COLOR_FG_DEFAULT,
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
            FF_COLOR_FG_DEFAULT,
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
            FF_COLOR_FG_DEFAULT,
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
    // NexaLinux
    {
        .names = {"nexalinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_NEXALINUX,
        .colors = {
            FF_COLOR_FG_LIGHT_BLUE,
            FF_COLOR_FG_LIGHT_BLUE,
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
        .names = {"NixOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // NixOSSmall
    {
        .names = {"NixOS_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // NixOSOld
    {
        .names = {"nixos_old"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS_OLD,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // NixOsOldSmall
    {
        .names = {"nixos_old_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS_OLD_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // NetBSD
    {
        .names = {"NetBSD"},
        .lines = FASTFETCH_DATATEXT_LOGO_NETBSD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // NetBSD2
    {
        .names = {"NetBSD2"},
        .lines = FASTFETCH_DATATEXT_LOGO_NETBSD2,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // NetBSD Small
    {
        .names = {"NetBSD_small"},
        .lines = FASTFETCH_DATATEXT_LOGO_NETBSD_SMALL,
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Nobara
    {
        .names = {"nobara", "nobara-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_NOBARA,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // NomadBSD
    {
        .names = {"nomadbsd"},
        .lines = FASTFETCH_DATATEXT_LOGO_NOMADBSD,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // NurOS
    {
        .names = {"NurOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_NUROS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
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
    // ObsidianOS
    {
        .names = {"ObsidianOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_OBSIDIANOS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_LIGHT_BLUE,
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
    // Opak
    {
        .names = {"Opak"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPAK,
        .colors = {}, // #1070
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // OpenBSDSmall
    {
        .names = {"openbsd_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_OPENBSD_SMALL,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
            FF_COLOR_FG_DEFAULT,
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
        .names = {"opensuse"},
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
        .names = {"opensuse_small"},
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
        .names = {"opensuse-microos"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_MICROOS,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
    },
    // OpenSuseLeap
    {
        .names = {"opensuse-leap"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_LEAP,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // OpenSuseLeapOld
    {
        .names = {"opensuse-leap_old"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_LEAP_OLD,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // OpenSuseTumbleweed
    {
        .names = {"opensuse-tumbleweed"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // OpenSuseTumbleweedSmall
    {
        .names = {"opensuse-tumbleweed_small"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED_SMALL,
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // OpenSuseTumbleweedOld
    {
        .names = {"opensuse-tumbleweed_old"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED_OLD,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // openSUSESlowroll
    {
        .names = {"opensuse-slowroll", "opensuse-tumbleweed-slowroll"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_SLOWROLL,
        .colors = {
            FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorKeys = FF_COLOR_FG_DEFAULT,
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
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // Oreon
    {
        .names = {"Oreon"},
        .lines = FASTFETCH_DATATEXT_LOGO_OREON,
        .colors = {
            FF_COLOR_FG_DEFAULT,
            FF_COLOR_FG_DEFAULT,
        },
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
    // PisiLinux
    {
        .names = {"PisiLinux"},
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
        .names = {"pop", "popos"},
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
        .names = {"pop_small", "popos_small"},
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
        .colorKeys = FF_COLOR_FG_DEFAULT,
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
    // PrismLinux
    {
        .names = {"PrismLinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_PRISMLINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
        },
    },
    // LAST
    {},
};

static const FFlogo Q[] = {
    // QTS
    {
        .names = {"qts"},
        .lines = FASTFETCH_DATATEXT_LOGO_QTS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
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
    // Quirinux
    {
        .names = {"Quirinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_QUIRINUX,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_MAGENTA,
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
    // RebornOS
    {
        .names = {"RebornOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_REBORNOS,
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
        .names = {"RebornOS_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_REBORNOS_SMALL,
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
        .names = {"rhel", "redhat"},
        .lines = FASTFETCH_DATATEXT_LOGO_RHEL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // RedHatEnterpriseLinux
    {
        .names = {"rhel_small", "redhat_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_RHEL_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // RedHatEnterpriseLinux_old
    {
        .names = {"rhel_old", "redhat_old"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_RHEL_OLD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // RedOS
    {
        .names = {"RedOS", "RED OS", "red-os", "redos"},
        .lines = FASTFETCH_DATATEXT_LOGO_REDOS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorTitle = FF_COLOR_FG_RED,
        .colorKeys = FF_COLOR_FG_RED,
    },
    // RedOS small
    {
        .names = {"RedOS_small", "RED OS_small", "red-os_small", "redos_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_REDOS_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorTitle = FF_COLOR_FG_RED,
        .colorKeys = FF_COLOR_FG_RED,
    },
    // RedstarOS
    {
        .names = {"redstar", "redstar-os", "redstaros"},
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
            FF_COLOR_FG_RGB "250;250;250",
            FF_COLOR_FG_RGB "100;165;225",
        },
        .colorKeys = FF_COLOR_FG_RGB "100;165;225",
        .colorTitle = FF_COLOR_FG_RGB "100;165;225",
    },
    // RhinoLinux
    {
        .names = {"Rhino Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_RHINO,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_LIGHT_BLUE,
            FF_COLOR_FG_LIGHT_MAGENTA,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
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
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
    // Secureblue
    {
        .names = {"secureblue"},
        .lines = FASTFETCH_DATATEXT_LOGO_SECUREBLUE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_DEFAULT,
        },
    },
    // Serpent OS
    {
        .names = {"Serpent OS"},
        .lines = FASTFETCH_DATATEXT_LOGO_SERPENT_OS,
        .colors = {
            FF_COLOR_FG_DEFAULT,
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
    // Shebang
    {
        .names = {"Shebang"},
        .lines = FASTFETCH_DATATEXT_LOGO_SHEBANG,
        .colors = {
            FF_COLOR_FG_WHITE,
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
    // SleeperOS
    {
        .names = {"SleeperOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_SLEEPEROS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        }
    },
    // SleeperOS
    {
        .names = {"SleeperOS_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_SLEEPEROS_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        }
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
    // SpoinkOS
    {
        .names = {"SpoinkOS", "spoink-os"},
        .lines = FASTFETCH_DATATEXT_LOGO_SPOINKOS,
        .colors = {
            FF_COLOR_FG_GREEN,
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
        .names = {"Slackware"},
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
        .names = {"Slackware_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_SLACKWARE_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
    // SnigdhaOS
    {
        .names = {"SnigdhaOS", "Snigdha"},
        .lines = FASTFETCH_DATATEXT_LOGO_SNIGDHAOS,
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
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Solaris
    {
        .names = {"solaris", "sunos"},
        .lines = FASTFETCH_DATATEXT_LOGO_SOLARIS,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // SolarisSmall
    {
        .names = {"solaris_small", "sunos_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_SOLARIS_SMALL,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Solus
    {
        .names = {"Solus", "solus-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_SOLUS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .names = {"SteamOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_STEAMOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Steam Deck
    {
        .names = {"SteamDeck"},
        .lines = FASTFETCH_DATATEXT_LOGO_STEAMDECK,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Steam Deck Small
    {
        .names = {"SteamDeck_small"},
        .lines = FASTFETCH_DATATEXT_LOGO_STEAMDECK_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Steam Deck OLED
    {
        .names = {"SteamDeckOled"},
        .lines = FASTFETCH_DATATEXT_LOGO_STEAMDECK,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
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
    // SummitOS
    {
        .names = {"SummitOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_SUMMITOS,
        .colors = {
            FF_COLOR_FG_RGB "143;191;80",
            FF_COLOR_FG_RGB "160;205;102",
            FF_COLOR_FG_RGB "181;225;102",
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
        .names = {"T2", "T2 SDE", "T2/Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_T2,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE,
        },
    },
    // T2Small
    {
        .names = {"T2_small", "T2 SDE_small", "T2/Linux_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_T2_SMALL,
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
    // TempleOS
    {
        .names = {"TempleOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_TEMPLEOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_CYAN,
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
    // Torizon OS
    {
        .names = {"Torizon OS", "TorizonCore"},
        .lines = FASTFETCH_DATATEXT_LOGO_TORIZONCORE,
        .colors = {
            FF_COLOR_FG_LIGHT_WHITE,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE
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
    // TrueNAS Scale
    {
        .names = {"TrueNAS-Scale"},
        .lines = FASTFETCH_DATATEXT_LOGO_TRUENAS_SCALE,
        .colors = {
            FF_COLOR_FG_256 "39",
            FF_COLOR_FG_256 "32",
            FF_COLOR_FG_256 "248",
        },
        .colorKeys = FF_COLOR_FG_256 "248",
        .colorTitle = FF_COLOR_FG_256 "32",
    },
    // TuxedoOS
    {
        .names = {"Tuxedo OS", "tuxedo"},
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
    // UBLinux
    {
        .names = {"UBLinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBLINUX,
        .colors = {
            FF_COLOR_FG_256 "38",
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_256 "38",
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // UBLinuxSmall
    {
        .names = {"UBLinux_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_UBLINUX_SMALL,
        .colors = {
            FF_COLOR_FG_256 "38",
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_256 "38",
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Ubuntu
    {
        .names = {"ubuntu", "ubuntu-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_RED,
        },
    },
    // UbuntuSmall
    {
        .names = {"ubuntu_small", "ubuntu-linux_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_RED,
        },
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
    // UbuntuOld2
    {
        .names = {"ubuntu_old2"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_OLD2,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // UbuntuOld2Small
    {
        .names = {"ubuntu_old2_small", "ubuntu_old2_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_OLD2_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
    // UbuntuGNOME
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
    // Ultramarine
    {
        .names = {"Ultramarine", "Ultramarine Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ULTRAMARINE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Ultramarine Small
    {
        .names = {"Ultramarine_small"},
        .lines = FASTFETCH_DATATEXT_LOGO_ULTRAMARINE_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Unifi
    {
        .names = {"Unifi"},
        .lines = FASTFETCH_DATATEXT_LOGO_UNIFI,
        .colors = {
            FF_COLOR_FG_WHITE,
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
        .colorKeys = FF_COLOR_FG_DEFAULT,
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
    // Valhalla
    {
        .names = {"Valhalla", "valhallaos", "valhalla-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_VALHALLA,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Vanilla
    {
        .names = {"vanilla"},
        .lines = FASTFETCH_DATATEXT_LOGO_VANILLA,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // Vanilla2
    {
        .names = {"vanilla2"},
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_VANILLA2,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // VanillaSmall
    {
        .names = {"vanilla_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_VANILLA_SMALL,
        .colors = {
            FF_COLOR_FG_LIGHT_YELLOW,
            FF_COLOR_FG_YELLOW,
        },
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
        .names = {"void"},
        .lines = FASTFETCH_DATATEXT_LOGO_VOID,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // VoidSmall
    {
        .names = {"void_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_VOID_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Void2
    {
        .names = {"void2"},
        .lines = FASTFETCH_DATATEXT_LOGO_VOID2,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_DEFAULT,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Void2Small
    {
        .names = {"void2_small"},
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_VOID2_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // LAST
    {},
};

static const FFlogo W[] = {
    // WiiLinux
    {
        .names = {"WiiLinuxNgx", "WiiLinux", "Wii-Linux", "Wii Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_WII_LINUX,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // Windows2025
    {
        .names = {"Windows Server 2025"},
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_2025,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_CYAN,
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
        .names = {"Windows 11_small"},
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
        .colorTitle = FF_COLOR_FG_DEFAULT,
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
    // WolfOS
    {
        .names = {"WolfOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_WOLFOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_GREEN,
        },
    },
    // LAST
    {},
};

static const FFlogo X[] = {
    // XCP-ng
    {
        .names = {"XCP-ng", "xenenterprise"},
        .lines = FASTFETCH_DATATEXT_LOGO_XCP_NG,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_YELLOW,
        }
    },
    // Xenia
    {
        .names = {"Xenia"},
        .lines = FASTFETCH_DATATEXT_LOGO_XENIA,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Xenia_old
    {
        .names = {"Xenia_old"},
        .lines = FASTFETCH_DATATEXT_LOGO_XENIA_OLD,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
        }
    },
    //XeroArch
    {
        .names = {"XeroArch"},
        .lines = FASTFETCH_DATATEXT_LOGO_XEROARCH,
        .colors = {
            FF_COLOR_FG_256 "50",
            FF_COLOR_FG_256 "14",
            FF_COLOR_FG_256 "50",
            FF_COLOR_FG_256 "93",
            FF_COLOR_FG_256 "16",
            FF_COLOR_FG_256 "15",
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
    // Xubuntu
    {
        .names = {"Xubuntu"},
        .lines = FASTFETCH_DATATEXT_LOGO_XUBUNTU,
        .colors = {
            FF_COLOR_FG_256 "25",
            FF_COLOR_FG_DEFAULT,
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
        .names = {"Zorin"},
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
