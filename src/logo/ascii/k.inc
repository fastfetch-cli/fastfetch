#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo K[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_KAISEN
    // Kaisen
    {
        .names = { "Kaisen" },
        .lines = FASTFETCH_DATATEXT_LOGO_KAISEN,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KALI
    // Kali
    {
        .names = { "Kali" },
        .lines = FASTFETCH_DATATEXT_LOGO_KALI,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KALI_SMALL
    // KaliSmall
    {
        .names = { "Kali_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_KALI_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KALPA_DESKTOP
    // Kalpa Desktop
    {
        .names = { "kalpa-desktop" },
        .lines = FASTFETCH_DATATEXT_LOGO_KALPA_DESKTOP,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KAOS
    // KaOS
    {
        .names = { "KaOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_KAOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KERNELOS
    // KernelOS
    {
        .names = { "KernelOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_KERNELOS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_MAGENTA,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KDELINUX
    // KDELinux
    {
        .names = { "kdelinux", "kde-linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_KDELINUX,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KDENEON
    // KDE Neon
    {
        .names = { "KDE Neon" }, // Distro ID is "neon"; Distro name is "KDE Neon"
        .lines = FASTFETCH_DATATEXT_LOGO_KDENEON,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_DEFAULT,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KIBAOS
    // KibaOS
    {
        .names = { "KibaOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_KIBAOS,
        .colors = {
            FF_COLOR_BG_WHITE,
            FF_COLOR_FG_BLUE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KIBOJOE
    // Kibojoe
    {
        .names = { "Kibojoe" },
        .lines = FASTFETCH_DATATEXT_LOGO_KIBOJOE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KISS
    // KISSLinux
    {
        .names = { "KISS", "kiss-linux", "kisslinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_KISS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KISS2
    // KISSLinux2
    {
        .names = { "kiss2" },
        .lines = FASTFETCH_DATATEXT_LOGO_KISS2,
        .colors = {
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KOGAION
    // Kogaion
    {
        .names = { "Kogaion" },
        .lines = FASTFETCH_DATATEXT_LOGO_KOGAION,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KORORA
    // Korora
    {
        .names = { "Korora" },
        .lines = FASTFETCH_DATATEXT_LOGO_KORORA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KRASSOS
    // KrassOS
    {
        .names = { "KrassOS", "Krass" },
        .lines = FASTFETCH_DATATEXT_LOGO_KRASSOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KSLINUX
    // KSLinux
    {
        .names = { "KSLinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_KSLINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KUBUNTU
    // Kubuntu
    {
        .names = { "Kubuntu", "kubuntu-linux", "kde-ubuntu" },
        .lines = FASTFETCH_DATATEXT_LOGO_KUBUNTU,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_KYLIN
    // Kylin
    {
        .names = { "Kylin" },
        .lines = FASTFETCH_DATATEXT_LOGO_KYLIN,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    // LAST
    {},
};
