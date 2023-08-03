#include "logo.h"
#include "logo_builtin.h"
#include "common/color.h"

const FFlogo ffLogoBuiltins[] = {
    // Unknown
    {
        .names = {"unknown", "question mark", "?"},
        .lines = FASTFETCH_DATATEXT_LOGO_UNKNOWN,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = "",
        .colorTitle = "",
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
            FF_COLOR_MODE_BOLD FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE,
            FF_COLOR_MODE_BOLD FF_COLOR_FG_GREEN,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_MODE_BOLD FF_COLOR_FG_YELLOW,
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
        .small = true,
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
        .small = true,
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
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_WHITE,
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
        .small = true,
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
    // ArchSmall
    {
        .names = {"arch_small", "archlinux_small", "arch-linux-small"},
        .small = true,
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
        .small = true,
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
        .small = true,
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
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // ArcoLinuxSmall
    {
        .names = {"arco_small", "arcolinux_small", "arco-linux_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_ARCO_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_GREEN,
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
    // BSD
    {
        .names = {"bsd"},
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
    // CachyOS
    {
        .names = {"cachy", "cachyos", "cachy-linux", "cachyos-linux"},
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
        .names = {"cachy_small", "cachyos_small", "cachy-linux-small", "cachyos-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_CACHYOS_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // CelOS
    {
        .names = {"cel", "celos", "cel-linux", "celos-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CELOS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // CentOS
    {
        .names = {"cent", "centos", "cent-linux", "centos-linux"},
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
        .names = {"cent_small", "centos_small", "cent-linux_small", "cent-linux-small", "centos-linux-small"},
        .small = true,
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
    // CRUX
    {
        .names = {"crux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CRUX,
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
        .names = {"crystal", "Crystal", "crystal-linux", "Crystal-Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CRYSTAL,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // Debian
    {
        .names = {"debian", "debian-linux"},
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
        .names = {"debian_small", "debian-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_DEBIAN_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Devuan
    {
        .names = {"devuan", "devuan-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_DEVUAN,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // DevuanSmall
    {
        .names = {"devuan_small", "devuan-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_DEVUAN_SMALL,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // Elementary
    {
        .names = {"elementary"},
        .lines = FASTFETCH_DATATEXT_LOGO_ELEMENTARY,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // ElementarySmall
    {
        .names = {"elementary_small", "elementary-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_ELEMENTARY_SMALL,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Deepin
    {
        .names = {"deepin", "deepin-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_DEEPIN,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Endeavour
    {
        .names = {"endeavour", "endeavour-linux", "endeavouros", "endeavouros-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ENDEAVOUR,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Enso
    {
        .names = {"enso", "uqc"},
        .lines = FASTFETCH_DATATEXT_LOGO_ENSO,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // Exherbo
    {
        .names = {"exherbo", "exherbo-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_EXHERBO,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Fedora
    {
        .names = {"fedora", "fedora-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // FedoraSmall
    {
        .names = {"fedora_small", "fedora-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // FedoraOld
    {
        .names = {"fedora_old", "fedora-old", "fedora-linux-old", "fedora-linux_old"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_OLD,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // FreeBSD
    {
        .names = {"freebsd", "HardenedBSD"},
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
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_FREEBSD_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Garuda
    {
        .names = {"garuda", "garuda-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_GARUDA,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // GarudaDragon
    {
        .names = {"garudadragon", "garuda-dragon", "garuda-linux-dragon"},
        .lines = FASTFETCH_DATATEXT_LOGO_GARUDA_DRAGON,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // GarudaSmall
    {
        .names = {"garuda_small", "garudalinux_small", "garuda-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_GARUDA_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Gentoo
    {
        .names = {"gentoo", "gentoo-linux"},
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
        .names = {"gentoo_small", "gentoo-linux-small"},
        .small = true,
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
        .names = {"ghostbsd"},
        .lines = FASTFETCH_DATATEXT_LOGO_GHOSTBSD,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_RED,
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
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_HAIKU_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // KDENeon
    {
        .names = {"kde", "kde-neon", "neon"},
        .lines = FASTFETCH_DATATEXT_LOGO_KDE,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // KISSLinux
    {
        .names = {"kiss", "kiss-linux", "kisslinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_KISS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Kubuntu
    {
        .names = {"kubuntu", "kubuntu-linux", "kde-ubuntu", "ubuntu-kde", "ubuntu-plasma"},
        .lines = FASTFETCH_DATATEXT_LOGO_KUBUNTU,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
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
    // Linux
    {
        .names = {"linux", "linux-generic"},
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
        .names = {"linux_small", "linux-generic_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_LINUX_SMALL,
        .colors = {
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
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
    // MacOS
    {
        .names = {"macos", "mac", "apple", "darwin", "osx"},
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
        .names = {"macos_small", "mac_small", "apple_small", "darwin_small", "osx_small"},
        .small = true,
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
        .names = {"macos2", "mac2", "apple2", "darwin2", "osx2"},
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
        .names = {"macos2_small", "mac2_small", "apple2_small", "darwin2_small", "osx2_small"},
        .small = true,
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
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_MANJARO_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Minix
    {
        .names = {"minix"},
        .lines = FASTFETCH_DATATEXT_LOGO_MINIX,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // Mint
    {
        .names = {"mint", "linuxmint", "mint-linux", "linux-mint"},
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
        .names = {"mint_small", "linuxmint_small", "mint-linux-small"},
        .small = true,
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
        .names = {"mint_old", "mint-old", "mint-linux_old", "mint-linux-old", "linux-mint_old", "linux-mint-old"},
        .lines = FASTFETCH_DATATEXT_LOGO_MINT_OLD,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Msys2
    {
        .names = {"msys2"},
        .lines = FASTFETCH_DATATEXT_LOGO_MSYS2,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_RED,
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
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_11_SMALL,
        .colors = {
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
    // NixOS
    {
        .names = {"nixos", "nix", "nixos-linux", "nix-linux", "nix-os", "nix_os", "nix_os_linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // NixOsOld
    {
        .names = {"nixos_old", "nix-old", "nixos-old", "nix_old", "nix-os-old", "nix_os_old"},
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS_OLD,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // NixOsSmall
    {
        .names = {"nixos_small", "nix-small", "nixos-small", "nix_small", "nix-os-small", "nix_os_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // MX
    {
        .names = {"mx"},
        .lines = FASTFETCH_DATATEXT_LOGO_MX,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // MXSmall
    {
        .names = {"mx_small", "mx-small"},
        .lines = FASTFETCH_DATATEXT_LOGO_MX_SMALL,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_CYAN,
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
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_OPENBSD_SMALL,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // OpenSuse
    {
        .names = {"suse", "opensuse", "open_suse", "open-suse", "suse-linux"},
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
        .names = {"suse_small", "opensuse_small", "open_suse_small", "open-suse_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
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
    // OpenSuseTumbleweed2
    {
        .names = {"opensuse_tumbleweed2"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED2,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // OpenMandriva
    {
        .names = {"openmandriva", "open-mandriva", "open_mandriva"},
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
    // Oracle
    {
        .names = {"oracle", "oracle linux"},
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
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_ORCHID_SMALL,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_MAGENTA,
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
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_PARABOLA_SMALL,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
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
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_POP_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },    // Raspbian
    {
        .names = {"raspbian", "raspi", "raspberrypi", "raspberrypios", "pios"},
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
        .names = {"raspbian_small", "raspi_small", "raspberrypi_small" "raspberrypios_small" "pios_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_RASPBIAN_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Reborn
    {
        .names = {"reborn", "reborn-os", "rebornos", "rebornos-linux", "reborn-os-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_REBORN,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // RebornSmall
    {
        .names = {"reborn_small", "reborn-os-small", "rebornos_small", "rebornos-linux-small", "reborn-os-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_REBORN_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
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
    // Proxmox
    {
        .names = {"proxmox"},
        .lines = FASTFETCH_DATATEXT_LOGO_PROXMOX,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_256 "202"
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_256 "202",
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
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_SLACKWARE_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
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
        .small = true,
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
        .names = {"ubuntu-budgie"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_BUDGIE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    // UbuntuGnome
    {
        .names = {"ubuntu-gnome"},
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
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_OLD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // UbuntuSmall
    {
        .names = {"ubuntu_small", "ubuntu-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Ubuntu2Small
    {
        .names = {"ubuntu2_small", "ubuntu2-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU2_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
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
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_VOID_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
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
};

const uint32_t ffLogoBuiltinLength = sizeof(ffLogoBuiltins) / sizeof(ffLogoBuiltins[0]);
