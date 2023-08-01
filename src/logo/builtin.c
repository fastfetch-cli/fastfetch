#include "logo.h"
#include "logo_builtin.h"

#define FF_LOGO_INIT static FFlogo logo; static bool init = false; if(init) return &logo; init = true;
// The names of small logo must end with `_small` or `-small`
#define FF_LOGO_INIT_SMALL FF_LOGO_INIT logo.small = true;
#define FF_LOGO_NAMES(...) static const char* names[] = (const char*[]) { __VA_ARGS__, NULL }; logo.names = names;
#define FF_LOGO_LINES(x) logo.data = x;
#define FF_LOGO_COLORS(...) static const char* colors[] = (const char*[]) { __VA_ARGS__, NULL }; logo.builtinColors = colors;
#define FF_LOGO_COLOR_KEYS(x) logo.colorKeys = x;
#define FF_LOGO_COLOR_TITLE(x) logo.colorTitle = x;
#define FF_LOGO_RETURN return &logo;

const FFlogo* ffLogoBuiltinGetUnknown(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("unknown", "question mark", "?")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_UNKNOWN)
    FF_LOGO_COLORS("")
    FF_LOGO_COLOR_KEYS("")
    FF_LOGO_COLOR_TITLE("")
    FF_LOGO_RETURN
}

static const FFlogo* getLogoAIX(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("aix")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_AIX)
    FF_LOGO_COLORS(
        "32", // green
        "37" // yellow
    )
    FF_LOGO_COLOR_KEYS("32"); // green
    FF_LOGO_COLOR_TITLE("37"); // yellow
    FF_LOGO_RETURN
}

static const FFlogo* getLogoAlmaLinux(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("almalinux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ALMALINUX)
    FF_LOGO_COLORS(
        "31", // red
        "1;33", // yellow
        "34", // blue
        "1;32", // light green
        "36" // cyan
    )
    FF_LOGO_COLOR_KEYS("1;33"); //yellow
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoAlpine(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("alpine", "alpinelinux", "alpine-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ALPINE)
    FF_LOGO_COLORS(
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("35"); //magenta
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoAlpineSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("alpine_small", "alpine-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ALPINE_SMALL)
    FF_LOGO_COLORS(
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("35"); //magenta
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoAndroid(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("android")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ANDROID)
    FF_LOGO_COLORS(
        "32", //green
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoAndroidSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("android-small", "android_small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ANDROID_SMALL)
    FF_LOGO_COLORS(
        "32" //green
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoArch(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("arch", "archlinux", "arch-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ARCH)
    FF_LOGO_COLORS(
        "36", //cyan
        "36" //cyan
    )
    FF_LOGO_COLOR_KEYS("36"); //cyan
    FF_LOGO_COLOR_TITLE("36"); //cyan
    FF_LOGO_RETURN
}

static const FFlogo* getLogoArchSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("arch_small", "archlinux_small", "arch-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ARCH_SMALL)
    FF_LOGO_COLORS(
        "36" //cyan
    )
    FF_LOGO_COLOR_KEYS("36"); //cyan
    FF_LOGO_COLOR_TITLE("36"); //cyan
    FF_LOGO_RETURN
}

static const FFlogo* getLogoArtix(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("artix", "artixlinux", "artix-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ARTIX)
    FF_LOGO_COLORS(
        "36" //cyan
    )
    FF_LOGO_COLOR_KEYS("36"); //cyan
    FF_LOGO_COLOR_TITLE("36"); //cyan
    FF_LOGO_RETURN
}

static const FFlogo* getLogoArtixSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("artix_small", "artixlinux_small", "artix-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ARTIX_SMALL)
    FF_LOGO_COLORS(
        "36" //cyan
    )
    FF_LOGO_COLOR_KEYS("36"); //cyan
    FF_LOGO_COLOR_TITLE("36"); //cyan
    FF_LOGO_RETURN
}

static const FFlogo* getLogoArcoLinux(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("arco", "arcolinux", "arco-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ARCO)
    FF_LOGO_COLORS(
        "34", //blue
        "32" //green
    )
    FF_LOGO_COLOR_KEYS("34"); //green
    FF_LOGO_COLOR_TITLE("34"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoAsahi(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("asahi", "asahi-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ASAHI)
    FF_LOGO_COLORS(
        "33", //yellow
        "32", //green
        "31", //red
        "38", //cyan
        "37", //magenta
        "36", //cyan
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("33"); //yellow
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoBSD(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("bsd")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_BSD)
    FF_LOGO_COLORS(
        "31",
        "37",
        "34",
        "33",
        "36"
    )
    FF_LOGO_COLOR_KEYS("31");
    FF_LOGO_COLOR_TITLE("37");
    FF_LOGO_RETURN
}

static const FFlogo* getLogoBedrock(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("bedrock", "bedrocklinux", "bedrock-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_BEDROCK)
    FF_LOGO_COLORS(
        "90", //grey
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("90"); //grey
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoCachyOS(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("cachy", "cachyos", "cachy-linux", "cachyos-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_CACHYOS)
    FF_LOGO_COLORS(
        "36", //cyan
        "32", //green
        "30"  //black
    )
    FF_LOGO_COLOR_KEYS("36"); //cyan
    FF_LOGO_COLOR_TITLE("36"); //cyan
    FF_LOGO_RETURN
}

static const FFlogo* getLogoCachyOSSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("cachy_small", "cachyos_small", "cachy-linux-small", "cachyos-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_CACHYOS_SMALL)
    FF_LOGO_COLORS(
        "36" //cyan
    )
    FF_LOGO_COLOR_KEYS("36"); //cyan
    FF_LOGO_COLOR_TITLE("36"); //cyan
    FF_LOGO_RETURN
}


static const FFlogo* getLogoCelOS(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("cel", "celos", "cel-linux", "celos-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_CELOS)
    FF_LOGO_COLORS(
        "35", //magenta
        "30" //black
    )
    FF_LOGO_COLOR_KEYS("36"); //cyan
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoCentOS(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("cent", "centos", "cent-linux", "centos-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_CENTOS)
    FF_LOGO_COLORS(
        "33", //yellow
        "32", //green
        "34", //blue
        "35", //magenta
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("33"); //yellow
    FF_LOGO_RETURN
}

static const FFlogo* getLogoCentOSSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("cent_small", "centos_small", "cent-linux_small", "cent-linux-small", "centos-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_CENTOS_SMALL)
    FF_LOGO_COLORS(
        "33", //yellow
        "32", //green
        "34", //blue
        "35" //magenta
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("33"); //yellow
    FF_LOGO_RETURN
}

static const FFlogo* getLogoCRUX(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("crux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_CRUX)
    FF_LOGO_COLORS(
        "34", //blue
        "35", //magenta
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("35");
    FF_LOGO_COLOR_TITLE("34");
    FF_LOGO_RETURN
}

static const FFlogo* getLogoCrystalLinux(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("crystal", "Crystal", "crystal-linux", "Crystal-Linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_CRYSTAL)
    FF_LOGO_COLORS(
        "35" //magenta
    )
    FF_LOGO_COLOR_KEYS("35"); //magenta
    FF_LOGO_COLOR_TITLE("35"); //magenta
    FF_LOGO_RETURN
}

static const FFlogo* getLogoDebian(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("debian", "debian-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_DEBIAN)
    FF_LOGO_COLORS(
        "31", //red
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoDebianSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("debian_small", "debian-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_DEBIAN_SMALL)
    FF_LOGO_COLORS(
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoDevuan(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("devuan", "devuan-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_DEVUAN)
    FF_LOGO_COLORS(
        "35" //magenta
    )
    FF_LOGO_COLOR_KEYS("35"); //magenta
    FF_LOGO_COLOR_TITLE("35"); //magenta
    FF_LOGO_RETURN
}

static const FFlogo* getLogoDevuanSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("devuan_small", "devuan-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_DEVUAN_SMALL)
    FF_LOGO_COLORS(
        "35" //magenta
    )
    FF_LOGO_COLOR_KEYS("35"); //magenta
    FF_LOGO_COLOR_TITLE("35"); //magenta
    FF_LOGO_RETURN
}

static const FFlogo* getLogoElementary(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("elementary")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ELEMENTARY)
    FF_LOGO_COLORS(
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoElementarySmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("elementary_small", "elementary-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ELEMENTARY_SMALL)
    FF_LOGO_COLORS(
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoDeepin(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("deepin", "deepin-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_DEEPIN)
    FF_LOGO_COLORS(
        "32" //green
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoEndeavour(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("endeavour", "endeavour-linux", "endeavouros", "endeavouros-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ENDEAVOUR)
    FF_LOGO_COLORS(
        "35", //magenta
        "31", //red
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("35"); //magenta
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoEnso(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("enso", "uqc")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ENSO)
    FF_LOGO_COLORS(
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("37"); //white
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoExherbo(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("exherbo", "exherbo-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_EXHERBO)
    FF_LOGO_COLORS(
        "34", //blue
        "37", //white
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoFedora(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("fedora", "fedora-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_FEDORA)
    FF_LOGO_COLORS(
        "34", //blue
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoFedoraSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("fedora_small", "fedora-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_FEDORA_SMALL)
    FF_LOGO_COLORS(
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoFedoraOld(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("fedora_old", "fedora-old", "fedora-linux-old", "fedora-linux_old")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_FEDORA_OLD)
    FF_LOGO_COLORS(
        "34", //blue
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoFreeBSD(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("freebsd")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_FREEBSD)
    FF_LOGO_COLORS(
        "37", //white
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("31") //red
    FF_LOGO_COLOR_TITLE("31") //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoFreeBSDSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("freebsd_small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_FREEBSD_SMALL)
    FF_LOGO_COLORS(
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("31") //red
    FF_LOGO_COLOR_TITLE("31") //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoGaruda(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("garuda", "garuda-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_GARUDA)
    FF_LOGO_COLORS(
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoGarudaDragon()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("garudadragon", "garuda-dragon", "garuda-linux-dragon")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_GARUDA_DRAGON)
    FF_LOGO_COLORS(
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoGarudaSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("garuda_small", "garudalinux_small", "garuda-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_GARUDA_SMALL)
    FF_LOGO_COLORS(
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoGentoo(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("gentoo", "gentoo-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_GENTOO)
    FF_LOGO_COLORS(
        "35", //magenta
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("35"); //magenta
    FF_LOGO_COLOR_TITLE("35"); //magenta
    FF_LOGO_RETURN
}

static const FFlogo* getLogoGentooSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("gentoo_small", "gentoo-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_GENTOO_SMALL)
    FF_LOGO_COLORS(
        "35", //magenta
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("35"); //magenta
    FF_LOGO_COLOR_TITLE("35"); //magenta
    FF_LOGO_RETURN
}

static const FFlogo* getLogoGhostBSD(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("ghostbsd")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_GHOSTBSD)
    FF_LOGO_COLORS(
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoGnome(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("Gnome")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_GNOME)
    FF_LOGO_COLORS(
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoGNU(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("GNU")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_GNU)
    FF_LOGO_COLORS(
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("37"); //white
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoHaiku(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("Haiku")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_HAIKU)
    FF_LOGO_COLORS(
        "31", //red
        "33", //yellow
        "37", //white
        "32" //green
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("33"); //yellow
    FF_LOGO_RETURN
}

static const FFlogo* getLogoHaikuSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("Haiku-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_HAIKU_SMALL)
    FF_LOGO_COLORS(
        "32" //green
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("33"); //yellow
    FF_LOGO_RETURN
}

static const FFlogo* getLogoKDENeon(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("kde", "kde-neon", "neon")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_KDE)
    FF_LOGO_COLORS(
        "32" //green
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoKISSLinux(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("kiss", "kiss-linux", "kisslinux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_KISS)
    FF_LOGO_COLORS(
        "35", //magenta
        "37", //white
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("35"); //magenta
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoKubuntu(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("kubuntu", "kubuntu-linux", "kde-ubuntu", "ubuntu-kde", "ubuntu-plasma")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_KUBUNTU)
    FF_LOGO_COLORS(
        "34", //blue
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoLangitKetujuh(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("langitketujuh", "l7")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_LANGITKETUJUH)
    FF_LOGO_COLORS(
        "34", //blue
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoLinux(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("linux", "linux-generic")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_LINUX)
    FF_LOGO_COLORS(
        "37", //white
        "30", //black
        "33" //yellow
    )
    FF_LOGO_COLOR_KEYS("37"); //white
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoLinuxSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("linux_small", "linux-generic_small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_LINUX_SMALL)
    FF_LOGO_COLORS(
        "30", //black
        "37", //white
        "33" //yellow
    )
    FF_LOGO_COLOR_KEYS("37"); //white
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoLMDE(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("LMDE")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_LMDE)
    FF_LOGO_COLORS(
        "32", //green
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoMacOS(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("macos", "mac", "apple", "darwin", "osx")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_MACOS)
    FF_LOGO_COLORS(
        "32", //green
        "33", //yellow
        "31", //red
        "35", //magenta
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("33"); //yellow
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoMacOSSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("macos_small", "mac_small", "apple_small", "darwin_small", "osx_small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_MACOS_SMALL)
    FF_LOGO_COLORS(
        "32", //green
        "33", //yellow
        "31", //red
        "35", //magenta
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("33"); //yellow
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoMacOS2(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("macos2", "mac2", "apple2", "darwin2", "osx2")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_MACOS2)
    FF_LOGO_COLORS(
        "32", //green
        "33", //yellow
        "31", //red
        "35", //magenta
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("33"); //yellow
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoMacOS2Small(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("macos2_small", "mac2_small", "apple2_small", "darwin2_small", "osx2_small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_MACOS2_SMALL)
    FF_LOGO_COLORS(
        "32", //green
        "33", //yellow
        "31", //red
        "35", //magenta
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("33"); //yellow
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoMandriva(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("mandriva", "mandrake")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_MANDRIVA)
    FF_LOGO_COLORS(
        "34", //blue
        "33" //yellow
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("33"); //yellow
    FF_LOGO_RETURN
}

static const FFlogo* getLogoManjaro(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("manjaro", "manjaro-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_MANJARO)
    FF_LOGO_COLORS(
        "32" //green
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoManjaroSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("manjaro_small", "manjaro-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_MANJARO_SMALL)
    FF_LOGO_COLORS(
        "32" //green
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoMinix(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("minix")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_MINIX)
    FF_LOGO_COLORS(
        "31", //red
        "37", //white
        "33" //yellow
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("33"); //yellow
    FF_LOGO_RETURN
}

static const FFlogo* getLogoMint(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("mint", "linuxmint", "mint-linux", "linux-mint")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_MINT)
    FF_LOGO_COLORS(
        "32", //green
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoMintSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("mint_small", "linuxmint_small", "mint-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_MINT_SMALL)
    FF_LOGO_COLORS(
        "32", //green
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoMintOld(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("mint_old", "mint-old", "mint-linux_old", "mint-linux-old", "linux-mint_old", "linux-mint-old")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_MINT_OLD)
    FF_LOGO_COLORS(
        "32", //green
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoMsys2(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("msys2")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_MSYS2)
    FF_LOGO_COLORS(
        "35", //magenta
        "37", //white
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("35"); //magenta
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoWindows11(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("Windows 11", "Windows Server 2022")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_WINDOWS_11)
    FF_LOGO_COLORS(
        "34", //blue
        "34", //blue
        "34", //blue
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("33"); //yellow
    FF_LOGO_COLOR_TITLE("36"); //cyan
    FF_LOGO_RETURN
}

static const FFlogo* getLogoWindows11Small(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("Windows 11_small", "Windows 11-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_WINDOWS_11_SMALL)
    FF_LOGO_COLORS(
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("33"); //yellow
    FF_LOGO_COLOR_TITLE("36"); //cyan
    FF_LOGO_RETURN
}

static const FFlogo* getLogoWindows8(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("Windows 8", "Windows 8.1", "Windows 10", "Windows Server 2012", "Windows Server 2012 R2", "Windows Server 2016", "Windows Server 2019")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_WINDOWS_8)
    FF_LOGO_COLORS(
        "36", //cyan
        "36", //cyan
        "36", //cyan
        "36" //cyan
    )
    FF_LOGO_COLOR_KEYS("33"); //yellow
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoWindows(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("Windows", "Windows 7", "Windows Server 2008", "Windows Server 2008 R2")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_WINDOWS)
    FF_LOGO_COLORS(
        "31", //red
        "32", //green
        "34", //blue
        "33" //yellow
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoWindows95(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("Windows 95", "Windows 9x")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_WINDOWS_95)
    FF_LOGO_COLORS(
        "36", //cyan
        "34", //blue
        "33", //yellow
        "32", //green
        "31",
        "30"
    )
    FF_LOGO_COLOR_KEYS("36"); //cyan
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoNixOS(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("nixos", "nix", "nixos-linux", "nix-linux", "nix-os", "nix_os", "nix_os_linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_NIXOS)
    FF_LOGO_COLORS(
        "34", //blue
        "36" //cyan
    )
    FF_LOGO_COLOR_KEYS("36"); //cyan
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoNixOsOld(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("nixos_old", "nix-old", "nixos-old", "nix_old", "nix-os-old", "nix_os_old")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_NIXOS_OLD)
    FF_LOGO_COLORS(
        "34", //blue
        "36" //cyan
    )
    FF_LOGO_COLOR_KEYS("36"); //cyan
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoNixOsSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("nixos_small", "nix-small", "nixos-small", "nix_small", "nix-os-small", "nix_os_small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_NIXOS_SMALL)
    FF_LOGO_COLORS(
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoMX(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("mx");
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_MX)
    FF_LOGO_COLORS(
        "37" // white
    )
    FF_LOGO_COLOR_KEYS("34"); // blue
    FF_LOGO_COLOR_TITLE("36"); // cyan
    FF_LOGO_RETURN
}

static const FFlogo* getLogoMXSmall(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("mx_small", "mx-small");
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_MX_SMALL)
    FF_LOGO_COLORS(
        "37" // white
    )
    FF_LOGO_COLOR_KEYS("34"); // blue
    FF_LOGO_COLOR_TITLE("36"); // cyan
    FF_LOGO_RETURN
}

static const FFlogo* getLogoNetBSD(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("netbsd");
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_NETBSD)
    FF_LOGO_COLORS(
        "35", // blue
        "37" // white
    )
    FF_LOGO_COLOR_KEYS("35"); // blue
    FF_LOGO_COLOR_TITLE("37"); // white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoNobara(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("nobara", "nobara-linux");
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_NOBARA)
    FF_LOGO_COLORS(
        "37" // white
    )
    FF_LOGO_COLOR_KEYS("37"); // white
    FF_LOGO_COLOR_TITLE("37"); // white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoNomadBSD(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("nomadbsd");
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_NOMADBSD)
    FF_LOGO_COLORS(
        "34" // blue
    )
    FF_LOGO_COLOR_KEYS("34"); // blue
    FF_LOGO_COLOR_TITLE("37"); // white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoOpenKylin(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("openkylin", "open-kylin");
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_OPENKYLIN)
    FF_LOGO_COLORS(
        "32", // cyan
        "37" // white
    )
    FF_LOGO_COLOR_KEYS("32"); // cyan
    FF_LOGO_COLOR_TITLE("37"); // white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoOpenBSD(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("openbsd");
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_OPENBSD)
    FF_LOGO_COLORS(
        "33", // blue
        "37", // white
        "36", // yellow
        "31" // red
    )
    FF_LOGO_COLOR_KEYS("33"); // blue
    FF_LOGO_COLOR_TITLE("37"); // white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoOpenBSDSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("openbsd_small", "openbsd-small");
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_OPENBSD_SMALL)
    FF_LOGO_COLORS(
        "33", // blue
        "37" // white
    )
    FF_LOGO_COLOR_KEYS("33"); // blue
    FF_LOGO_COLOR_TITLE("37"); // white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoOpenSuse(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("suse", "opensuse", "open_suse", "open-suse", "suse-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_OPENSUSE)
    FF_LOGO_COLORS(
        "32", //green
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoOpenSuseSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("suse_small", "opensuse_small", "open_suse_small", "open-suse_small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_OPENSUSE_SMALL)
    FF_LOGO_COLORS(
        "32" //green
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoOpenSuseLeap(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("opensuse_leap", "open_suse_leap", "opensuse-leap", "open-suse-leap", "suse_leap", "suse-leap", "opensuseleap")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_OPENSUSE_LEAP)
    FF_LOGO_COLORS(
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoOpenSuseTumbleweed(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("opensuse_tumbleweed", "open_suse_tumbleweed", "opensuse-tumbleweed", "open-suse-tumbleweed", "suse_tumbleweed", "suse-tumbleweed", "opensusetumbleweed")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED)
    FF_LOGO_COLORS(
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoOpenSuseTumbleweed2(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("opensuse_tumbleweed2")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED2)
    FF_LOGO_COLORS(
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoOpenMandriva(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("openmandriva", "open-mandriva", "open_mandriva")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_OPENMANDRIVA)
    FF_LOGO_COLORS(
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoOpenWrt(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("openwrt")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_OPENWRT)
    FF_LOGO_COLORS(
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoOracle(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("oracle", "oracle linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ORACLE)
    FF_LOGO_COLORS(
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoOrchid(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("orchid")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ORCHID)
    FF_LOGO_COLORS(
        "37", //white
        "35", //magenta
        "35" //magenta
    )
    FF_LOGO_COLOR_KEYS("37"); //white
    FF_LOGO_COLOR_TITLE("35"); //magenta
    FF_LOGO_RETURN
}

static const FFlogo* getLogoOrchidSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("orchid_small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ORCHID_SMALL)
    FF_LOGO_COLORS(
        "37", //white
        "35", //magenta
        "35" //magenta
    )
    FF_LOGO_COLOR_KEYS("37"); //white
    FF_LOGO_COLOR_TITLE("35"); //magenta
    FF_LOGO_RETURN
}

static const FFlogo* getLogoParabola(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("parabola", "parabola-gnulinux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_PARABOLA)
    FF_LOGO_COLORS(
        "35" //magenta
    )
    FF_LOGO_COLOR_KEYS("35"); //magenta
    FF_LOGO_COLOR_TITLE("35"); //magenta
    FF_LOGO_RETURN
}

static const FFlogo* getLogoParabolaSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("parabola_small", "parabola-gnulinux_small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_PARABOLA_SMALL)
    FF_LOGO_COLORS(
        "35" //magenta
    )
    FF_LOGO_COLOR_KEYS("35"); //magenta
    FF_LOGO_COLOR_TITLE("35"); //magenta
    FF_LOGO_RETURN
}

static const FFlogo* getLogoPop(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("pop", "popos", "pop_os", "pop-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_POP)
    FF_LOGO_COLORS(
        "36", //cyan
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("36"); //cyan
    FF_LOGO_COLOR_TITLE("36"); //cyan
    FF_LOGO_RETURN
}

static const FFlogo* getLogoPopSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("pop_small", "popos_small", "pop_os_small", "pop-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_POP_SMALL)
    FF_LOGO_COLORS(
        "36" //cyan
    )
    FF_LOGO_COLOR_KEYS("36"); //cyan
    FF_LOGO_COLOR_TITLE("36"); //cyan
    FF_LOGO_RETURN
}
static const FFlogo* getLogoRaspbian(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("raspbian", "raspi", "raspberrypi", "raspberrypios", "pios")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_RASPBIAN)
    FF_LOGO_COLORS(
        "31", //red
        "32" //green
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoRaspbianSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("raspbian_small", "raspi_small", "raspberrypi_small" "raspberrypios_small" "pios_small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_RASPBIAN_SMALL)
    FF_LOGO_COLORS(
        "31", //red
        "32" //green
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoReborn(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("reborn", "reborn-os", "rebornos", "rebornos-linux", "reborn-os-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_REBORN)
    FF_LOGO_COLORS(
        "34", //blue
        "36" //cyan
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoRebornSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("reborn_small", "reborn-os-small", "rebornos_small", "rebornos-linux-small", "reborn-os-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_REBORN_SMALL)
    FF_LOGO_COLORS(
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoRedHatEnterpriseLinux(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("rhel", "redhat", "redhat-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_RHEL)
    FF_LOGO_COLORS(
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoRedstarOS(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("redstar", "redstar-os", "redstaros", "redstaros-linux", "redstar-os-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_REDSTAR)
    FF_LOGO_COLORS(
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoRockyLinux(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("rocky", "rocky-linux", "rockylinux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ROCKY)
    FF_LOGO_COLORS(
        "32" //green
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoRosaLinux(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("rosa", "rosa-linux", "rosalinux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ROSA)
    FF_LOGO_COLORS(
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoProxmox(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("proxmox")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_PROXMOX)
    FF_LOGO_COLORS(
        "37", //blue
        "38;5;202"
    )
    FF_LOGO_COLOR_KEYS("37"); //white
    FF_LOGO_COLOR_TITLE("38;5;202"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoSlackware(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("slackware", "slackware-linux", "slackwarelinux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_SLACKWARE)
    FF_LOGO_COLORS(
        "34", //blue
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("34") //blue
    FF_LOGO_COLOR_TITLE("34") //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoSlackwareSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("slackware-small", "slackware-linux-small", "slackware_small", "slackwarelinux_small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_SLACKWARE_SMALL)
    FF_LOGO_COLOR_KEYS("33"); //green
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoSolaris(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("solaris")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_SOLARIS)
    FF_LOGO_COLORS(
        "33" //green
    )
    FF_LOGO_COLOR_KEYS("33"); //green
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoSolarisSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("solaris-small", "solaris_small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_SOLARIS_SMALL)
    FF_LOGO_COLORS(
        "33" //green
    )
    FF_LOGO_COLOR_KEYS("33"); //green
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoSolus(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("solus", "solus-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_SOLUS)
    FF_LOGO_COLORS(
        "34", //blue
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoSteamOS(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("steamos")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_STEAMOS)
    FF_LOGO_COLORS(
        "34", //blue
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

static const FFlogo* getLogoUbuntu(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("ubuntu", "ubuntu-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_UBUNTU)
    FF_LOGO_COLORS(
        "31", //red
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoUbuntuBudgie(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("ubuntu-budgie")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_UBUNTU_BUDGIE)
    FF_LOGO_COLORS(
        "34", //blue
        "37", //white
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoUbuntuGnome(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("ubuntu-gnome")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_UBUNTU_GNOME)
    FF_LOGO_COLORS(
        "34", //blue
        "35", //magenta
        "37", //white
        "36" //cyan
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("35"); //magenta
    FF_LOGO_RETURN
}

static const FFlogo* getLogoUbuntuKylin(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("ubuntu kylin", "ubuntu-kylin")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_UBUNTU_KYLIN)
    FF_LOGO_COLORS(
        "31", //red
        "37", //white
        "33" //yellow
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoUbuntuMate(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("ubuntu mate", "ubuntu-mate")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_UBUNTU_MATE)
    FF_LOGO_COLORS(
        "32", //green
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("32"); //green
    FF_LOGO_COLOR_TITLE("37"); //white
    FF_LOGO_RETURN
}

static const FFlogo* getLogoUbuntuOld(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("ubuntu_old", "ubuntu-linux_old")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_UBUNTU_OLD)
    FF_LOGO_COLORS(
        "31", //red
        "37" //white
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoUbuntuSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("ubuntu_small", "ubuntu-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_UBUNTU_SMALL)
    FF_LOGO_COLORS(
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoUbuntu2Small(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("ubuntu2_small", "ubuntu2-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_UBUNTU2_SMALL)
    FF_LOGO_COLORS(
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("31"); //red
    FF_LOGO_COLOR_TITLE("31"); //red
    FF_LOGO_RETURN
}

static const FFlogo* getLogoUOS(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("UOS")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_UOS)
    FF_LOGO_COLORS(
        "31" //red
    )
    FF_LOGO_COLOR_KEYS("37"); //white
    FF_LOGO_COLOR_TITLE("33"); //yellow
    FF_LOGO_RETURN
}

static const FFlogo* getLogoVanilla(void)
{

    FF_LOGO_INIT
    FF_LOGO_NAMES("vanilla", "vanilla-os", "vanilla-linux");
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_VANILLA)
    FF_LOGO_COLORS(
        "33" //yellow
    )
    FF_LOGO_COLOR_KEYS("33"); // yellow
    FF_LOGO_COLOR_TITLE("33"); // yellow
    FF_LOGO_RETURN
}

static const FFlogo* getLogoVoid(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("void", "void-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_VOID)
    FF_LOGO_COLORS(
        "32", //green
        "30" //black
    )
    FF_LOGO_COLOR_KEYS("37"); //white
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoVoidSmall(void)
{
    FF_LOGO_INIT_SMALL
    FF_LOGO_NAMES("void_small", "void-linux-small")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_VOID_SMALL)
    FF_LOGO_COLORS(
        "32" //green
    )
    FF_LOGO_COLOR_KEYS("37"); //white
    FF_LOGO_COLOR_TITLE("32"); //green
    FF_LOGO_RETURN
}

static const FFlogo* getLogoZorin(void)
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("zorin", "zorin-linux", "zorinos", "zorinos-linux")
    FF_LOGO_LINES(FASTFETCH_DATATEXT_LOGO_ZORIN)
    FF_LOGO_COLORS(
        "34" //blue
    )
    FF_LOGO_COLOR_KEYS("34"); //blue
    FF_LOGO_COLOR_TITLE("34"); //blue
    FF_LOGO_RETURN
}

GetLogoMethod* ffLogoBuiltinGetAll(void)
{
    static GetLogoMethod logoMethods[] = {
        ffLogoBuiltinGetUnknown,
        getLogoAIX,
        getLogoAlmaLinux,
        getLogoAlpine,
        getLogoAlpineSmall,
        getLogoAndroid,
        getLogoAndroidSmall,
        getLogoArch,
        getLogoArchSmall,
        getLogoArcoLinux,
        getLogoArtix,
        getLogoArtixSmall,
        getLogoAsahi,
        getLogoBedrock,
        getLogoBSD,
        getLogoCachyOS,
        getLogoCachyOSSmall,
        getLogoCelOS,
        getLogoCentOS,
        getLogoCentOSSmall,
        getLogoCRUX,
        getLogoCrystalLinux,
        getLogoElementary,
        getLogoElementarySmall,
        getLogoDebian,
        getLogoDebianSmall,
        getLogoDevuan,
        getLogoDevuanSmall,
        getLogoDeepin,
        getLogoEndeavour,
        getLogoEnso,
        getLogoExherbo,
        getLogoFedora,
        getLogoFedoraSmall,
        getLogoFedoraOld,
        getLogoFreeBSD,
        getLogoFreeBSDSmall,
        getLogoGaruda,
        getLogoGarudaDragon,
        getLogoGarudaSmall,
        getLogoGentoo,
        getLogoGentooSmall,
        getLogoGhostBSD,
        getLogoGnome,
        getLogoGNU,
        getLogoHaiku,
        getLogoHaikuSmall,
        getLogoKDENeon,
        getLogoKISSLinux,
        getLogoKubuntu,
        getLogoLangitKetujuh,
        getLogoLinux,
        getLogoLinuxSmall,
        getLogoLMDE,
        getLogoMacOS,
        getLogoMacOSSmall,
        getLogoMacOS2,
        getLogoMacOS2Small,
        getLogoMandriva,
        getLogoManjaro,
        getLogoManjaroSmall,
        getLogoMinix,
        getLogoMint,
        getLogoMintSmall,
        getLogoMintOld,
        getLogoMsys2,
        getLogoWindows11,
        getLogoWindows11Small,
        getLogoWindows8,
        getLogoWindows,
        getLogoWindows95,
        getLogoNixOS,
        getLogoNixOsOld,
        getLogoNixOsSmall,
        getLogoNomadBSD,
        getLogoMX,
        getLogoMXSmall,
        getLogoNetBSD,
        getLogoNobara,
        getLogoOpenBSD,
        getLogoOpenBSDSmall,
        getLogoOpenKylin,
        getLogoOpenSuse,
        getLogoOpenSuseSmall,
        getLogoOpenSuseLeap,
        getLogoOpenSuseTumbleweed,
        getLogoOpenSuseTumbleweed2,
        getLogoOpenMandriva,
        getLogoOpenWrt,
        getLogoOrchid,
        getLogoOrchidSmall,
        getLogoOracle,
        getLogoPop,
        getLogoPopSmall,
        getLogoParabola,
        getLogoParabolaSmall,
        getLogoProxmox,
        getLogoRaspbian,
        getLogoRaspbianSmall,
        getLogoReborn,
        getLogoRebornSmall,
        getLogoRedHatEnterpriseLinux,
        getLogoRedstarOS,
        getLogoRockyLinux,
        getLogoRosaLinux,
        getLogoSolaris,
        getLogoSolarisSmall,
        getLogoSlackware,
        getLogoSlackwareSmall,
        getLogoSolus,
        getLogoSteamOS,
        getLogoUbuntu,
        getLogoUbuntuBudgie,
        getLogoUbuntuGnome,
        getLogoUbuntuKylin,
        getLogoUbuntuMate,
        getLogoUbuntuOld,
        getLogoUbuntuSmall,
        getLogoUbuntu2Small,
        getLogoUOS,
        getLogoVanilla,
        getLogoVoid,
        getLogoVoidSmall,
        getLogoZorin,
        NULL
    };

    return logoMethods;
}
