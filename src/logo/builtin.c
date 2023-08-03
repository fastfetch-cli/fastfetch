#include "logo.h"
#include "logo_builtin.h"

const FFlogo ffLogoBuiltins[] = {
    // Unknown
    {
        .names = {"unknown", "question mark", "?"},
        .lines = FASTFETCH_DATATEXT_LOGO_UNKNOWN,
        .colors = {
            "37", //white
        },
        .colorKeys = "",
        .colorTitle = "",
    },
    // AIX
    {
        .names = {"aix"},
        .lines = FASTFETCH_DATATEXT_LOGO_AIX,
        .colors = {
            "32", // green
            "37" // white
        },
        .colorKeys = "32", // green
        .colorTitle = "37", // white
    },
    // AlmaLinux
    {
        .names = {"almalinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ALMALINUX,
        .colors = {
            "31", // red
            "1;33", // yellow
            "34", // blue
            "1;32", // light green
            "36" // cyan
        },
        .colorKeys = "1;33", //yellow
        .colorTitle = "31", //red
    },
    // Alpine
    {
        .names = {"alpine", "alpinelinux", "alpine-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ALPINE,
        .colors = {
            "34" //blue
        },
        .colorKeys = "35", //magenta
        .colorTitle = "34", //blue
    },
    // AlpineSmall
    {
        .names = {"alpine_small", "alpine-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_ALPINE_SMALL,
        .colors = {
            "34", //blue
            "37" //white
        },
        .colorKeys = "35", //magenta
        .colorTitle = "34", //blue
    },
    // Alpine2Small
    {
        .names = {"alpine2_small", "alpine-linux2-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_ALPINE2_SMALL,
        .colors = {
            "34", //blue
            "37" //white
        },
        .colorKeys = "35", //magenta
        .colorTitle = "34", //blue
    },
    // Alter
    {
        .names = {"Alter"},
        .lines = FASTFETCH_DATATEXT_LOGO_ALTER,
        .colors = {
            "36" //cyan
        },
        .colorKeys = "36", //cyan
        .colorTitle = "36", //cyan
    },
    // Amazon
    {
        .names = {"Amazon"},
        .lines = FASTFETCH_DATATEXT_LOGO_AMAZON,
        .colors = {
            "33", //yellow
            "37" //white
        },
        .colorKeys = "33", //yellow
        .colorTitle = "37", //white
    },
    // AmogOS
    {
        .names = {"AmogOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_AMOGOS,
        .colors = {
            "37", //white
            "36" //cyan
        },
        .colorKeys = "37", //white
        .colorTitle = "36", //cyan
    },
    // Anarchy
    {
        .names = {"Anarchy"},
        .lines = FASTFETCH_DATATEXT_LOGO_ANARCHY,
        .colors = {
            "37", //white
            "34" //blue
        },
        .colorKeys = "37", //white
        .colorTitle = "34", //blue
    },
    // Android
    {
        .names = {"android"},
        .lines = FASTFETCH_DATATEXT_LOGO_ANDROID,
        .colors = {
            "32", //green
            "37" //white
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // AndroidSmall
    {
        .names = {"android-small", "android_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_ANDROID_SMALL,
        .colors = {
            "32" //green
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // Antergos
    {
        .names = {"Antergos"},
        .lines = FASTFETCH_DATATEXT_LOGO_ANTERGOS,
        .colors = {
            "34", //blue
            "36" //cyan
        },
        .colorKeys = "34", //blue
        .colorTitle = "36", //cyan
    },
    // Antix
    {
        .names = {"antiX"},
        .lines = FASTFETCH_DATATEXT_LOGO_ANTIX,
        .colors = {
            "31" //red
        },
        .colorKeys = "31", //red
        .colorTitle = "37", //white
    },
    // AoscOsRetro
    {
        .names = {"Aosc OS/Retro", "aoscosretro"},
        .lines = FASTFETCH_DATATEXT_LOGO_AOSCOSRETRO,
        .colors = {
            "34", //blue
            "37" //white
        },
        .colorKeys = "34", //blue
        .colorTitle = "37", //white
    },
    // AoscOsRetro_small
    {
        .names = {"Aosc OS/Retro_small", "aoscosretro_small"},
        .lines = FASTFETCH_DATATEXT_LOGO_AOSCOSRETRO_SMALL,
        .colors = {
            "34", //blue
            "37" //white
        },
        .colorKeys = "34", //blue
        .colorTitle = "37", //white
    },
    // AoscOS
    {
        .names = {"Aosc OS", "aoscos"},
        .lines = FASTFETCH_DATATEXT_LOGO_AOSCOS,
        .colors = {
            "34", //blue
            "37" //white
        },
        .colorKeys = "34", //blue
        .colorTitle = "37", //white
    },
    // Aperture
    {
        .names = {"Aperture"},
        .lines = FASTFETCH_DATATEXT_LOGO_APERTURE,
        .colors = {
            "36" //cyan
        },
        .colorKeys = "36", //cyan
        .colorTitle = "37", //white
    },
    // Apricity
    {
        .names = {"Apricity"},
        .lines = FASTFETCH_DATATEXT_LOGO_APRICITY,
        .colors = {
            "37" //white
        },
        .colorKeys = "34", //blue
        .colorTitle = "37", //white
    },
    // ArchBox
    {
        .names = {"ArchBox"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARCHBOX,
        .colors = {
            "32" //green
        },
        .colorKeys = "32", //green
        .colorTitle = "37", //white
    },
    // Archcraft
    {
        .names = {"Archcraft"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARCHCRAFT,
        .colors = {
            "36" //cyan
        },
        .colorKeys = "36", //cyan
        .colorTitle = "31", //red
    },
    // Archcraft2
    {
        .names = {"Archcraft2"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARCHCRAFT2,
        .colors = {
            "36" //cyan
        },
        .colorKeys = "36", //cyan
        .colorTitle = "31", //red
    },
    // Arch
    {
        .names = {"arch", "archlinux", "arch-linux", "archmerge"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH,
        .colors = {
            "36", //cyan
            "36" //cyan
        },
        .colorKeys = "36", //cyan
        .colorTitle = "36", //cyan
    },
    // ArchSmall
    {
        .names = {"arch_small", "archlinux_small", "arch-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_ARCH_SMALL,
        .colors = {
            "36" //cyan
        },
        .colorKeys = "36", //cyan
        .colorTitle = "36", //cyan
    },
    // Archlabs
    {
        .names = {"ARCHlabs"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARCHLABS,
        .colors = {
            "36", //cyan
            "31" //red
        },
        .colorKeys = "36", //cyan
        .colorTitle = "31", //red
    },
    // ArchStrike
    {
        .names = {"ArchStrike"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARCHSTRIKE,
        .colors = {
            "36", //cyan
            "30" //black
        },
        .colorKeys = "36", //cyan
        .colorTitle = "36", //cyan
    },
    // Artix
    {
        .names = {"artix", "artixlinux", "artix-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARTIX,
        .colors = {
            "36" //cyan
        },
        .colorKeys = "36", //cyan
        .colorTitle = "36", //cyan
    },
    // ArtixSmall
    {
        .names = {"artix_small", "artixlinux_small", "artix-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_ARTIX_SMALL,
        .colors = {
            "36" //cyan
        },
        .colorKeys = "36", //cyan
        .colorTitle = "36", //cyan
    },
    // Artix2Small
    {
        .names = {"artix2_small", "artixlinux2_small", "artix-linux2-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_ARTIX2_SMALL,
        .colors = {
            "36" //cyan
        },
        .colorKeys = "36", //cyan
        .colorTitle = "36", //cyan
    },
    // ArcoLinux
    {
        .names = {"arco", "arcolinux", "arco-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARCO,
        .colors = {
            "34", //blue
            "32" //green
        },
        .colorKeys = "34", //green
        .colorTitle = "34", //green
    },
    // ArcoLinuxSmall
    {
        .names = {"arco_small", "arcolinux_small", "arco-linux_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_ARCO_SMALL,
        .colors = {
            "34", //blue
            "32" //green
        },
        .colorKeys = "34", //green
        .colorTitle = "34", //green
    },
    // ArseLinux
    {
        .names = {"arse", "arselinux", "arse-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARSELINUX,
        .colors = {
            "34", //blue
            "37" //white
        },
        .colorKeys = "34", //green
        .colorTitle = "37", //white
    },
    // Arya
    {
        .names = {"Arya"},
        .lines = FASTFETCH_DATATEXT_LOGO_ARYA,
        .colors = {
            "32", //green
            "31" //red
        },
        .colorKeys = "32", //green
        .colorTitle = "31", //red
    },
    // Asahi
    {
        .names = {"asahi", "asahi-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ASAHI,
        .colors = {
            "33", //yellow
            "32", //green
            "31", //red
            "38", //cyan
            "37", //magenta
            "36", //cyan
            "34" //blue
        },
        .colorKeys = "33", //yellow
        .colorTitle = "32", //green
    },
    // Aster
    {
        .names = {"aster"},
        .lines = FASTFETCH_DATATEXT_LOGO_ASTER,
        .colors = {
            "36" //cyan
        },
        .colorKeys = "36", //cyan
        .colorTitle = "36", //cyan
    },
    // AsteroidOS
    {
        .names = {"AsteroidOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_ASTEROIDOS,
        .colors = {
            "38;5;160",
            "38;5;208",
            "38;5;202",
            "38;5;214"
        },
        .colorKeys = "38;5;160",
        .colorTitle = "38;5;208",
    },
    // AstOS
    {
        .names = {"astOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_ASTOS,
        .colors = {
            "37" //white
        },
        .colorKeys = "37", //white
        .colorTitle = "37", //white
    },
    // BSD
    {
        .names = {"bsd"},
        .lines = FASTFETCH_DATATEXT_LOGO_BSD,
        .colors = {
            "31",
            "37",
            "34",
            "33",
            "36"
        },
        .colorKeys = "31",
        .colorTitle = "37",
    },
    // Bedrock
    {
        .names = {"bedrock", "bedrocklinux", "bedrock-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_BEDROCK,
        .colors = {
            "90", //grey
            "37" //white
        },
        .colorKeys = "90", //grey
        .colorTitle = "37", //white
    },
    // CachyOS
    {
        .names = {"cachy", "cachyos", "cachy-linux", "cachyos-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CACHYOS,
        .colors = {
            "36", //cyan
            "32", //green
            "30"  //black
        },
        .colorKeys = "36", //cyan
        .colorTitle = "36", //cyan
    },
    // CachyOSSmall
    {
        .names = {"cachy_small", "cachyos_small", "cachy-linux-small", "cachyos-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_CACHYOS_SMALL,
        .colors = {
            "36" //cyan
        },
        .colorKeys = "36", //cyan
        .colorTitle = "36", //cyan
    },
    // CelOS
    {
        .names = {"cel", "celos", "cel-linux", "celos-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CELOS,
        .colors = {
            "35", //magenta
            "30" //black
        },
        .colorKeys = "36", //cyan
        .colorTitle = "34", //blue
    },
    // CentOS
    {
        .names = {"cent", "centos", "cent-linux", "centos-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CENTOS,
        .colors = {
            "33", //yellow
            "32", //green
            "34", //blue
            "35", //magenta
            "37" //white
        },
        .colorKeys = "32", //green
        .colorTitle = "33", //yellow
    },
    // CentOSSmall
    {
        .names = {"cent_small", "centos_small", "cent-linux_small", "cent-linux-small", "centos-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_CENTOS_SMALL,
        .colors = {
            "33", //yellow
            "32", //green
            "34", //blue
            "35" //magenta
        },
        .colorKeys = "32", //green
        .colorTitle = "33", //yellow
    },
    // CRUX
    {
        .names = {"crux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CRUX,
        .colors = {
            "34", //blue
            "35", //magenta
            "37" //white
        },
        .colorKeys = "35",
        .colorTitle = "34",
    },
    // CrystalLinux
    {
        .names = {"crystal", "Crystal", "crystal-linux", "Crystal-Linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_CRYSTAL,
        .colors = {
            "35" //magenta
        },
        .colorKeys = "35", //magenta
        .colorTitle = "35", //magenta
    },
    // Debian
    {
        .names = {"debian", "debian-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_DEBIAN,
        .colors = {
            "31", //red
            "37" //white
        },
        .colorKeys = "31", //red
        .colorTitle = "31", //red
    },
    // DebianSmall
    {
        .names = {"debian_small", "debian-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_DEBIAN_SMALL,
        .colors = {
            "31" //red
        },
        .colorKeys = "31", //red
        .colorTitle = "31", //red
    },
    // Devuan
    {
        .names = {"devuan", "devuan-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_DEVUAN,
        .colors = {
            "35" //magenta
        },
        .colorKeys = "35", //magenta
        .colorTitle = "35", //magenta
    },
    // DevuanSmall
    {
        .names = {"devuan_small", "devuan-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_DEVUAN_SMALL,
        .colors = {
            "35" //magenta
        },
        .colorKeys = "35", //magenta
        .colorTitle = "35", //magenta
    },
    // Elementary
    {
        .names = {"elementary"},
        .lines = FASTFETCH_DATATEXT_LOGO_ELEMENTARY,
        .colors = {
            "37" //white
        },
        .colorKeys = "34", //blue
        .colorTitle = "37", //white
    },
    // ElementarySmall
    {
        .names = {"elementary_small", "elementary-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_ELEMENTARY_SMALL,
        .colors = {
            "37" //white
        },
        .colorKeys = "34", //blue
        .colorTitle = "37", //white
    },
    // Deepin
    {
        .names = {"deepin", "deepin-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_DEEPIN,
        .colors = {
            "32" //green
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // Endeavour
    {
        .names = {"endeavour", "endeavour-linux", "endeavouros", "endeavouros-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ENDEAVOUR,
        .colors = {
            "35", //magenta
            "31", //red
            "34" //blue
        },
        .colorKeys = "35", //magenta
        .colorTitle = "31", //red
    },
    // Enso
    {
        .names = {"enso", "uqc"},
        .lines = FASTFETCH_DATATEXT_LOGO_ENSO,
        .colors = {
            "37" //white
        },
        .colorKeys = "37", //white
        .colorTitle = "37", //white
    },
    // Exherbo
    {
        .names = {"exherbo", "exherbo-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_EXHERBO,
        .colors = {
            "34", //blue
            "37", //white
            "31" //red
        },
        .colorKeys = "34", //blue
        .colorTitle = "34", //blue
    },
    // Fedora
    {
        .names = {"fedora", "fedora-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA,
        .colors = {
            "34", //blue
            "37" //white
        },
        .colorKeys = "34", //blue
        .colorTitle = "34", //blue
    },
    // FedoraSmall
    {
        .names = {"fedora_small", "fedora-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_SMALL,
        .colors = {
            "34" //blue
        },
        .colorKeys = "34", //blue
        .colorTitle = "34", //blue
    },
    // FedoraOld
    {
        .names = {"fedora_old", "fedora-old", "fedora-linux-old", "fedora-linux_old"},
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_OLD,
        .colors = {
            "34", //blue
            "37" //white
        },
        .colorKeys = "34", //blue
        .colorTitle = "34", //blue
    },
    // FreeBSD
    {
        .names = {"freebsd", "HardenedBSD"},
        .lines = FASTFETCH_DATATEXT_LOGO_FREEBSD,
        .colors = {
            "37", //white
            "31" //red
        },
        .colorKeys = "31", //red
        .colorTitle = "31", //red
    },
    // FreeBSDSmall
    {
        .names = {"freebsd_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_FREEBSD_SMALL,
        .colors = {
            "31" //red
        },
        .colorKeys = "31", //red
        .colorTitle = "31", //red
    },
    // Garuda
    {
        .names = {"garuda", "garuda-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_GARUDA,
        .colors = {
            "31" //red
        },
        .colorKeys = "31", //red
        .colorTitle = "31", //red
    },
    // GarudaDragon
    {
        .names = {"garudadragon", "garuda-dragon", "garuda-linux-dragon"},
        .lines = FASTFETCH_DATATEXT_LOGO_GARUDA_DRAGON,
        .colors = {
            "31" //red
        },
        .colorKeys = "31", //red
        .colorTitle = "31", //red
    },
    // GarudaSmall
    {
        .names = {"garuda_small", "garudalinux_small", "garuda-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_GARUDA_SMALL,
        .colors = {
            "31" //red
        },
        .colorKeys = "31", //red
        .colorTitle = "31", //red
    },
    // Gentoo
    {
        .names = {"gentoo", "gentoo-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_GENTOO,
        .colors = {
            "35", //magenta
            "37" //white
        },
        .colorKeys = "35", //magenta
        .colorTitle = "35", //magenta
    },
    // GentooSmall
    {
        .names = {"gentoo_small", "gentoo-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_GENTOO_SMALL,
        .colors = {
            "35", //magenta
            "37" //white
        },
        .colorKeys = "35", //magenta
        .colorTitle = "35", //magenta
    },
    // GhostBSD
    {
        .names = {"ghostbsd"},
        .lines = FASTFETCH_DATATEXT_LOGO_GHOSTBSD,
        .colors = {
            "34" //blue
        },
        .colorKeys = "34", //blue
        .colorTitle = "31", //red
    },
    // Gnome
    {
        .names = {"Gnome"},
        .lines = FASTFETCH_DATATEXT_LOGO_GNOME,
        .colors = {
            "34" //blue
        },
        .colorKeys = "34", //blue
        .colorTitle = "31", //red
    },
    // GNU
    {
        .names = {"GNU"},
        .lines = FASTFETCH_DATATEXT_LOGO_GNU,
        .colors = {
            "37" //white
        },
        .colorKeys = "37", //white
        .colorTitle = "31", //red
    },
    // Haiku
    {
        .names = {"Haiku"},
        .lines = FASTFETCH_DATATEXT_LOGO_HAIKU,
        .colors = {
            "31", //red
            "33", //yellow
            "37", //white
            "32" //green
        },
        .colorKeys = "31", //red
        .colorTitle = "33", //yellow
    },
    // HaikuSmall
    {
        .names = {"Haiku-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_HAIKU_SMALL,
        .colors = {
            "32" //green
        },
        .colorKeys = "32", //green
        .colorTitle = "33", //yellow
    },
    // KDENeon
    {
        .names = {"kde", "kde-neon", "neon"},
        .lines = FASTFETCH_DATATEXT_LOGO_KDE,
        .colors = {
            "32" //green
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // KISSLinux
    {
        .names = {"kiss", "kiss-linux", "kisslinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_KISS,
        .colors = {
            "35", //magenta
            "37", //white
            "34" //blue
        },
        .colorKeys = "35", //magenta
        .colorTitle = "34", //blue
    },
    // Kubuntu
    {
        .names = {"kubuntu", "kubuntu-linux", "kde-ubuntu", "ubuntu-kde", "ubuntu-plasma"},
        .lines = FASTFETCH_DATATEXT_LOGO_KUBUNTU,
        .colors = {
            "34", //blue
            "37" //white
        },
        .colorKeys = "34", //blue
        .colorTitle = "34", //blue
    },
    // LangitKetujuh
    {
        .names = {"langitketujuh", "l7"},
        .lines = FASTFETCH_DATATEXT_LOGO_LANGITKETUJUH,
        .colors = {
            "34", //blue
            "37" //white
        },
        .colorKeys = "34", //blue
        .colorTitle = "34", //blue
    },
    // Linux
    {
        .names = {"linux", "linux-generic"},
        .lines = FASTFETCH_DATATEXT_LOGO_LINUX,
        .colors = {
            "37", //white
            "30", //black
            "33" //yellow
        },
        .colorKeys = "37", //white
        .colorTitle = "37", //white
    },
    // LinuxSmall
    {
        .names = {"linux_small", "linux-generic_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_LINUX_SMALL,
        .colors = {
            "30", //black
            "37", //white
            "33" //yellow
        },
        .colorKeys = "37", //white
        .colorTitle = "37", //white
    },
    // LMDE
    {
        .names = {"LMDE"},
        .lines = FASTFETCH_DATATEXT_LOGO_LMDE,
        .colors = {
            "32", //green
            "37" //white
        },
        .colorKeys = "32", //green
        .colorTitle = "37", //white
    },
    // MacOS
    {
        .names = {"macos", "mac", "apple", "darwin", "osx"},
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS,
        .colors = {
            "32", //green
            "33", //yellow
            "31", //red
            "35", //magenta
            "34" //blue
        },
        .colorKeys = "33", //yellow
        .colorTitle = "32", //green
    },
    // MacOSSmall
    {
        .names = {"macos_small", "mac_small", "apple_small", "darwin_small", "osx_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS_SMALL,
        .colors = {
            "32", //green
            "33", //yellow
            "31", //red
            "35", //magenta
            "34" //blue
        },
        .colorKeys = "33", //yellow
        .colorTitle = "32", //green
    },
    // MacOS2
    {
        .names = {"macos2", "mac2", "apple2", "darwin2", "osx2"},
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS2,
        .colors = {
            "32", //green
            "33", //yellow
            "31", //red
            "35", //magenta
            "34" //blue
        },
        .colorKeys = "33", //yellow
        .colorTitle = "32", //green
    },
    // MacOS2Small
    {
        .names = {"macos2_small", "mac2_small", "apple2_small", "darwin2_small", "osx2_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS2_SMALL,
        .colors = {
            "32", //green
            "33", //yellow
            "31", //red
            "35", //magenta
            "34" //blue
        },
        .colorKeys = "33", //yellow
        .colorTitle = "32", //green
    },
    // Mandriva
    {
        .names = {"mandriva", "mandrake"},
        .lines = FASTFETCH_DATATEXT_LOGO_MANDRIVA,
        .colors = {
            "34", //blue
            "33" //yellow
        },
        .colorKeys = "34", //blue
        .colorTitle = "33", //yellow
    },
    // Manjaro
    {
        .names = {"manjaro", "manjaro-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_MANJARO,
        .colors = {
            "32" //green
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // ManjaroSmall
    {
        .names = {"manjaro_small", "manjaro-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_MANJARO_SMALL,
        .colors = {
            "32" //green
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // Minix
    {
        .names = {"minix"},
        .lines = FASTFETCH_DATATEXT_LOGO_MINIX,
        .colors = {
            "31", //red
            "37", //white
            "33" //yellow
        },
        .colorKeys = "31", //red
        .colorTitle = "33", //yellow
    },
    // Mint
    {
        .names = {"mint", "linuxmint", "mint-linux", "linux-mint"},
        .lines = FASTFETCH_DATATEXT_LOGO_MINT,
        .colors = {
            "32", //green
            "37" //white
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // MintSmall
    {
        .names = {"mint_small", "linuxmint_small", "mint-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_MINT_SMALL,
        .colors = {
            "32", //green
            "37" //white
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // MintOld
    {
        .names = {"mint_old", "mint-old", "mint-linux_old", "mint-linux-old", "linux-mint_old", "linux-mint-old"},
        .lines = FASTFETCH_DATATEXT_LOGO_MINT_OLD,
        .colors = {
            "32", //green
            "37" //white
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // Msys2
    {
        .names = {"msys2"},
        .lines = FASTFETCH_DATATEXT_LOGO_MSYS2,
        .colors = {
            "35", //magenta
            "37", //white
            "31" //red
        },
        .colorKeys = "35", //magenta
        .colorTitle = "31", //red
    },
    // Windows11
    {
        .names = {"Windows 11", "Windows Server 2022"},
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_11,
        .colors = {
            "34", //blue
            "34", //blue
            "34", //blue
            "34" //blue
        },
        .colorKeys = "33", //yellow
        .colorTitle = "36", //cyan
    },
    // Windows11Small
    {
        .names = {"Windows 11_small", "Windows 11-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_11_SMALL,
        .colors = {
            "34" //blue
        },
        .colorKeys = "33", //yellow
        .colorTitle = "36", //cyan
    },
    // Windows8
    {
        .names = {"Windows 8", "Windows 8.1", "Windows 10", "Windows Server 2012", "Windows Server 2012 R2", "Windows Server 2016", "Windows Server 2019"},
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_8,
        .colors = {
            "36", //cyan
            "36", //cyan
            "36", //cyan
            "36" //cyan
        },
        .colorKeys = "33", //yellow
        .colorTitle = "37", //white
    },
    // Windows
    {
        .names = {"Windows", "Windows 7", "Windows Server 2008", "Windows Server 2008 R2"},
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS,
        .colors = {
            "31", //red
            "32", //green
            "34", //blue
            "33" //yellow
        },
        .colorKeys = "34", //blue
        .colorTitle = "32", //green
    },
    // Windows95
    {
        .names = {"Windows 95", "Windows 9x"},
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_95,
        .colors = {
            "36", //cyan
            "34", //blue
            "33", //yellow
            "32", //green
            "31", //red
            "30" //black
        },
        .colorKeys = "36", //cyan
        .colorTitle = "34", //blue
    },
    // NixOS
    {
        .names = {"nixos", "nix", "nixos-linux", "nix-linux", "nix-os", "nix_os", "nix_os_linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS,
        .colors = {
            "34", //blue
            "36" //cyan
        },
        .colorKeys = "36", //cyan
        .colorTitle = "34", //blue
    },
    // NixOsOld
    {
        .names = {"nixos_old", "nix-old", "nixos-old", "nix_old", "nix-os-old", "nix_os_old"},
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS_OLD,
        .colors = {
            "34", //blue
            "36" //cyan
        },
        .colorKeys = "36", //cyan
        .colorTitle = "34", //blue
    },
    // NixOsSmall
    {
        .names = {"nixos_small", "nix-small", "nixos-small", "nix_small", "nix-os-small", "nix_os_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS_SMALL,
        .colors = {
            "34" //blue
        },
        .colorKeys = "34", //blue
        .colorTitle = "34", //blue
    },
    // MX
    {
        .names = {"mx"},
        .lines = FASTFETCH_DATATEXT_LOGO_MX,
        .colors = {
            "37" // white
        },
        .colorKeys = "34", // blue
        .colorTitle = "36", // cyan
    },
    // MXSmall
    {
        .names = {"mx_small", "mx-small"},
        .lines = FASTFETCH_DATATEXT_LOGO_MX_SMALL,
        .colors = {
            "37" // white
        },
        .colorKeys = "34", // blue
        .colorTitle = "36", // cyan
    },
    // NetBSD
    {
        .names = {"netbsd"},
        .lines = FASTFETCH_DATATEXT_LOGO_NETBSD,
        .colors = {
            "35", // blue
            "37" // white
        },
        .colorKeys = "35", // blue
        .colorTitle = "37", // white
    },
    // Nobara
    {
        .names = {"nobara", "nobara-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_NOBARA,
        .colors = {
            "37" // white
        },
        .colorKeys = "37", // white
        .colorTitle = "37", // white
    },
    // NomadBSD
    {
        .names = {"nomadbsd"},
        .lines = FASTFETCH_DATATEXT_LOGO_NOMADBSD,
        .colors = {
            "34" // blue
        },
        .colorKeys = "34", // blue
        .colorTitle = "37", // white
    },
    // OpenKylin
    {
        .names = {"openkylin", "open-kylin"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENKYLIN,
        .colors = {
            "32", // cyan
            "37" // white
        },
        .colorKeys = "32", // cyan
        .colorTitle = "37", // white
    },
    // OpenBSD
    {
        .names = {"openbsd"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENBSD,
        .colors = {
            "33", // blue
            "37", // white
            "36", // yellow
            "31" // red
        },
        .colorKeys = "33", // blue
        .colorTitle = "37", // white
    },
    // OpenBSDSmall
    {
        .names = {"openbsd_small", "openbsd-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_OPENBSD_SMALL,
        .colors = {
            "33", // blue
            "37" // white
        },
        .colorKeys = "33", // blue
        .colorTitle = "37", // white
    },
    // OpenSuse
    {
        .names = {"suse", "opensuse", "open_suse", "open-suse", "suse-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE,
        .colors = {
            "32", //green
            "37" //white
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // OpenSuseSmall
    {
        .names = {"suse_small", "opensuse_small", "open_suse_small", "open-suse_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_SMALL,
        .colors = {
            "32" //green
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // OpenSuseLeap
    {
        .names = {"opensuse_leap", "open_suse_leap", "opensuse-leap", "open-suse-leap", "suse_leap", "suse-leap", "opensuseleap"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_LEAP,
        .colors = {
            "37" //white
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // OpenSuseTumbleweed
    {
        .names = {"opensuse_tumbleweed", "open_suse_tumbleweed", "opensuse-tumbleweed", "open-suse-tumbleweed", "suse_tumbleweed", "suse-tumbleweed", "opensusetumbleweed"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED,
        .colors = {
            "37" //white
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // OpenSuseTumbleweed2
    {
        .names = {"opensuse_tumbleweed2"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED2,
        .colors = {
            "37" //white
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // OpenMandriva
    {
        .names = {"openmandriva", "open-mandriva", "open_mandriva"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENMANDRIVA,
        .colors = {
            "34" //blue
        },
        .colorKeys = "34", //blue
        .colorTitle = "34", //blue
    },
    // OpenWrt
    {
        .names = {"openwrt"},
        .lines = FASTFETCH_DATATEXT_LOGO_OPENWRT,
        .colors = {
            "34" //blue
        },
        .colorKeys = "34", //blue
        .colorTitle = "37", //white
    },
    // Oracle
    {
        .names = {"oracle", "oracle linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ORACLE,
        .colors = {
            "31" //red
        },
        .colorKeys = "31", //red
        .colorTitle = "37", //white
    },
    // Orchid
    {
        .names = {"orchid"},
        .lines = FASTFETCH_DATATEXT_LOGO_ORCHID,
        .colors = {
            "37", //white
            "35", //magenta
            "35" //magenta
        },
        .colorKeys = "37", //white
        .colorTitle = "35", //magenta
    },
    // OrchidSmall
    {
        .names = {"orchid_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_ORCHID_SMALL,
        .colors = {
            "37", //white
            "35", //magenta
            "35" //magenta
        },
        .colorKeys = "37", //white
        .colorTitle = "35", //magenta
    },
    // Parabola
    {
        .names = {"parabola", "parabola-gnulinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_PARABOLA,
        .colors = {
            "35" //magenta
        },
        .colorKeys = "35", //magenta
        .colorTitle = "35", //magenta
    },
    // ParabolaSmall
    {
        .names = {"parabola_small", "parabola-gnulinux_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_PARABOLA_SMALL,
        .colors = {
            "35" //magenta
        },
        .colorKeys = "35", //magenta
        .colorTitle = "35", //magenta
    },
    // Pop
    {
        .names = {"pop", "popos", "pop_os", "pop-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_POP,
        .colors = {
            "36", //cyan
            "37" //white
        },
        .colorKeys = "36", //cyan
        .colorTitle = "36", //cyan
    },
    // PopSmall
    {
        .names = {"pop_small", "popos_small", "pop_os_small", "pop-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_POP_SMALL,
        .colors = {
            "36" //cyan
        },
        .colorKeys = "36", //cyan
        .colorTitle = "36", //cyan
    },    // Raspbian
    {
        .names = {"raspbian", "raspi", "raspberrypi", "raspberrypios", "pios"},
        .lines = FASTFETCH_DATATEXT_LOGO_RASPBIAN,
        .colors = {
            "31", //red
            "32" //green
        },
        .colorKeys = "31", //red
        .colorTitle = "32", //green
    },
    // RaspbianSmall
    {
        .names = {"raspbian_small", "raspi_small", "raspberrypi_small" "raspberrypios_small" "pios_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_RASPBIAN_SMALL,
        .colors = {
            "31", //red
            "32" //green
        },
        .colorKeys = "31", //red
        .colorTitle = "32", //green
    },
    // Reborn
    {
        .names = {"reborn", "reborn-os", "rebornos", "rebornos-linux", "reborn-os-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_REBORN,
        .colors = {
            "34", //blue
            "36" //cyan
        },
        .colorKeys = "34", //blue
        .colorTitle = "34", //blue
    },
    // RebornSmall
    {
        .names = {"reborn_small", "reborn-os-small", "rebornos_small", "rebornos-linux-small", "reborn-os-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_REBORN_SMALL,
        .colors = {
            "34" //blue
        },
        .colorKeys = "34", //blue
        .colorTitle = "34", //blue
    },
    // RedHatEnterpriseLinux
    {
        .names = {"rhel", "redhat", "redhat-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_RHEL,
        .colors = {
            "31" //red
        },
        .colorKeys = "31", //red
        .colorTitle = "31", //red
    },
    // RedstarOS
    {
        .names = {"redstar", "redstar-os", "redstaros", "redstaros-linux", "redstar-os-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_REDSTAR,
        .colors = {
            "31" //red
        },
        .colorKeys = "31", //red
        .colorTitle = "31", //red
    },
    // RockyLinux
    {
        .names = {"rocky", "rocky-linux", "rockylinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ROCKY,
        .colors = {
            "32" //green
        },
        .colorKeys = "32", //green
        .colorTitle = "32", //green
    },
    // RosaLinux
    {
        .names = {"rosa", "rosa-linux", "rosalinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ROSA,
        .colors = {
            "34" //blue
        },
        .colorKeys = "34", //blue
        .colorTitle = "34", //blue
    },
    // Proxmox
    {
        .names = {"proxmox"},
        .lines = FASTFETCH_DATATEXT_LOGO_PROXMOX,
        .colors = {
            "37", //blue
            "38;5;202"
        },
        .colorKeys = "37", //white
        .colorTitle = "38;5;202", //white
    },
    // Slackware
    {
        .names = {"slackware", "slackware-linux", "slackwarelinux"},
        .lines = FASTFETCH_DATATEXT_LOGO_SLACKWARE,
        .colors = {
            "34", //blue
            "37" //white
        },
        .colorKeys = "34", //blue
        .colorTitle = "34", //blue
    },
    // SlackwareSmall
    {
        .names = {"slackware-small", "slackware-linux-small", "slackware_small", "slackwarelinux_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_SLACKWARE_SMALL,
        .colors = {
            "34", //blue
            "37" //white
        },
        .colorKeys = "34", //blue
        .colorTitle = "37", //white
    },
    // Solaris
    {
        .names = {"solaris", "sunos"},
        .lines = FASTFETCH_DATATEXT_LOGO_SOLARIS,
        .colors = {
            "33" //green
        },
        .colorKeys = "33", //green
        .colorTitle = "37", //white
    },
    // SolarisSmall
    {
        .names = {"solaris-small", "solaris_small", "sunos-small", "sunos_small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_SOLARIS_SMALL,
        .colors = {
            "33" //green
        },
        .colorKeys = "33", //green
        .colorTitle = "37", //white
    },
    // Solus
    {
        .names = {"solus", "solus-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_SOLUS,
        .colors = {
            "34", //blue
            "37" //white
        },
        .colorKeys = "34", //blue
        .colorTitle = "37", //white
    },
    // SteamOS
    {
        .names = {"steamos"},
        .lines = FASTFETCH_DATATEXT_LOGO_STEAMOS,
        .colors = {
            "34", //blue
            "37" //white
        },
        .colorKeys = "34", //blue
        .colorTitle = "34", //blue
    },
    // Ubuntu
    {
        .names = {"ubuntu", "ubuntu-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU,
        .colors = {
            "31", //red
            "37" //white
        },
        .colorKeys = "31", //red
        .colorTitle = "31", //red
    },
    // UbuntuBudgie
    {
        .names = {"ubuntu-budgie"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_BUDGIE,
        .colors = {
            "34", //blue
            "37", //white
            "31" //red
        },
        .colorKeys = "34", //blue
        .colorTitle = "37", //white
    },
    // UbuntuGnome
    {
        .names = {"ubuntu-gnome"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_GNOME,
        .colors = {
            "34", //blue
            "35", //magenta
            "37", //white
            "36" //cyan
        },
        .colorKeys = "34", //blue
        .colorTitle = "35", //magenta
    },
    // UbuntuKylin
    {
        .names = {"ubuntu kylin", "ubuntu-kylin"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_KYLIN,
        .colors = {
            "31", //red
            "37", //white
            "33" //yellow
        },
        .colorKeys = "31", //red
        .colorTitle = "37", //white
    },
    // UbuntuMate
    {
        .names = {"ubuntu mate", "ubuntu-mate"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_MATE,
        .colors = {
            "32", //green
            "37" //white
        },
        .colorKeys = "32", //green
        .colorTitle = "37", //white
    },
    // UbuntuOld
    {
        .names = {"ubuntu_old", "ubuntu-linux_old"},
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_OLD,
        .colors = {
            "31", //red
            "37" //white
        },
        .colorKeys = "31", //red
        .colorTitle = "31", //red
    },
    // UbuntuSmall
    {
        .names = {"ubuntu_small", "ubuntu-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_SMALL,
        .colors = {
            "31" //red
        },
        .colorKeys = "31", //red
        .colorTitle = "31", //red
    },
    // Ubuntu2Small
    {
        .names = {"ubuntu2_small", "ubuntu2-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU2_SMALL,
        .colors = {
            "31" //red
        },
        .colorKeys = "31", //red
        .colorTitle = "31", //red
    },
    // UOS
    {
        .names = {"UOS"},
        .lines = FASTFETCH_DATATEXT_LOGO_UOS,
        .colors = {
            "31" //red
        },
        .colorKeys = "37", //white
        .colorTitle = "33", //yellow
    },
    // Vanilla
    {
        .names = {"vanilla", "vanilla-os", "vanilla-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_VANILLA,
        .colors = {
            "33" //yellow
        },
        .colorKeys = "33", // yellow
        .colorTitle = "33", // yellow
    },
    // Void
    {
        .names = {"void", "void-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_VOID,
        .colors = {
            "32", //green
            "30" //black
        },
        .colorKeys = "37", //white
        .colorTitle = "32", //green
    },
    // VoidSmall
    {
        .names = {"void_small", "void-linux-small"},
        .small = true,
        .lines = FASTFETCH_DATATEXT_LOGO_VOID_SMALL,
        .colors = {
            "32" //green
        },
        .colorKeys = "37", //white
        .colorTitle = "32", //green
    },
    // Zorin
    {
        .names = {"zorin", "zorin-linux", "zorinos", "zorinos-linux"},
        .lines = FASTFETCH_DATATEXT_LOGO_ZORIN,
        .colors = {
            "34" //blue
        },
        .colorKeys = "34", //blue
        .colorTitle = "34", //blue
    },
};

const uint32_t ffLogoBuiltinLength = sizeof(ffLogoBuiltins) / sizeof(ffLogoBuiltins[0]);
