#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo K[] = {
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
    // KernelOS
    {
        .names = { "KernelOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_KERNELOS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_MAGENTA,
        },
    },
    // KDELinux
    {
        .names = { "kdelinux", "kde-linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_KDELINUX,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    // KDE Neon
    {
        .names = { "KDE Neon" }, // Distro ID is "neon"; Distro name is "KDE Neon"
        .lines = FASTFETCH_DATATEXT_LOGO_KDENEON,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_DEFAULT,
        },
    },
    // KibaOS
    {
        .names = { "KibaOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_KIBAOS,
        .colors = {
            FF_COLOR_BG_WHITE,
            FF_COLOR_FG_BLUE,
        },
    },
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
    // LAST
    {},
};
