#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo M[] = {
    // Macaroni
    {
        .names = { "Macaroni" },
        .lines = FASTFETCH_DATATEXT_LOGO_MACARONIOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // MacOS
    {
        .names = { "macOS" },
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
    // MacOSSmall
    {
        .names = { "macOS_small" },
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
        .names = { "macOS2" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS2,
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
    // MacOS2Small
    {
        .names = { "macOS2_small" },
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
        .names = { "macOS3" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS3,
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
    // MainsailOS
    {
        .names = { "MainsailOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_MAINSAILOS,
        .colors = {
            FF_COLOR_FG_RED,
        },
    },
    // MainsailOSSmall
    {
        .names = { "MainsailOS_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MAINSAILOS_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
    },
    // Mageia
    {
        .names = { "Mageia" },
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
        .names = { "Mageia_small" },
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
        .names = { "Magix", "MagixOS" },
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
        .names = { "MagpieOS", "Magpie" },
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
        .names = { "mandriva", "mandrake" },
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
        .names = { "manjaro", "manjaro-arm" },
        .lines = FASTFETCH_DATATEXT_LOGO_MANJARO,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // ManjaroSmall
    {
        .names = { "manjaro_small", "manjaro-arm_small" },
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
        .names = { "MassOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_MASSOS,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // MatuusOS
    {
        .names = { "MatuusOS", "Matuus" },
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
        .names = { "MaUI" },
        .lines = FASTFETCH_DATATEXT_LOGO_MAUI,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Mauna
    {
        .names = { "Mauna" },
        .lines = FASTFETCH_DATATEXT_LOGO_MAUNA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // Meowix
    {
        .names = { "Meowix" },
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
        .names = { "Mer" },
        .lines = FASTFETCH_DATATEXT_LOGO_MER,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // MidnightBSD
    {
        .names = { "MidnightBSD" },
        .lines = FASTFETCH_DATATEXT_LOGO_MIDNIGHTBSD,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // MidOS
    {
        .names = { "MidOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_MIDOS,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // MidOSOld
    {
        .names = { "MidOS_old" },
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
        .names = { "Minimal_System" },
        .lines = FASTFETCH_DATATEXT_LOGO_MINIMAL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Minix
    {
        .names = { "Minix" },
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
        .names = { "miraclelinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_MIRACLE_LINUX,
        .colors = {
            FF_COLOR_FG_256 "29",
        },
        .colorKeys = FF_COLOR_FG_256 "29",
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // MOS
    {
        .names = { "MOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_MOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
        },
    },
    // Msys2
    {
        .names = { "Msys2" },
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
        .names = { "MX" },
        .lines = FASTFETCH_DATATEXT_LOGO_MX,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // MXSmall
    {
        .names = { "MX_small" },
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
        .names = { "MX2" },
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
