#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo L[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_LAINOS
    // LainOS
    {
        .names = { "LainOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_LAINOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_256 "14",
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LANGITKETUJUH
    // LangitKetujuh
    {
        .names = { "langitketujuh", "l7" },
        .lines = FASTFETCH_DATATEXT_LOGO_LANGITKETUJUH,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LAXEROS
    // Laxeros
    {
        .names = { "Laxeros" },
        .lines = FASTFETCH_DATATEXT_LOGO_LAXEROS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LEDE
    // LEDE
    {
        .names = { "LEDE" },
        .lines = FASTFETCH_DATATEXT_LOGO_LEDE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LIBREELEC
    // LibreELEC
    {
        .names = { "LibreELEC" },
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LILIDOG
    // Lilidog
    {
        .names = { "Lilidog" },
        .lines = FASTFETCH_DATATEXT_LOGO_LILIDOG,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LINGMO
    // Lingmo OS
    {
        .names = { "Lingmo", "LingmoOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_LINGMO,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LIMEOS
    // LimeOS
    {
        .names = { "LimeOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_LIMEOS,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LINSPIRE
    // Linspire
    {
        .names = { "Linspire", "Lindows" },
        .lines = FASTFETCH_DATATEXT_LOGO_LINSPIRE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LINUX
    // Linux
    {
        .names = { "Linux", "linux-generic" },
        .lines = FASTFETCH_DATATEXT_LOGO_LINUX,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LFS
    // LinuxFromScratch
    {
        .names = { "LinuxFromScratch", "lfs" },
        .lines = FASTFETCH_DATATEXT_LOGO_LFS,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_BLACK,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LINUX_SMALL
    // LinuxSmall
    {
        .names = { "Linux_small", "linux-generic_small" },
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LINUXLITE
    // LinuxLite
    {
        .names = { "LinuxLite" },
        .lines = FASTFETCH_DATATEXT_LOGO_LINUXLITE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LINUXLITE_SMALL
    // LinuxLiteSmall
    {
        .names = { "LinuxLite_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_LINUXLITE_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LINUXMINT
    // LinuxMint
    {
        .names = { "linuxmint" },
        .lines = FASTFETCH_DATATEXT_LOGO_LINUXMINT,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LINUXMINT_SMALL
    // LinuxMintSmall
    {
        .names = { "linuxmint_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_LINUXMINT_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LINUXMINT2
    // LinuxMint2
    {
        .names = { "linuxmint2" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_LINUXMINT2,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LINUXMINT_OLD
    // LinuxMintOld
    {
        .names = { "linuxmint_old" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_LINUXMINT_OLD,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LIVE_RAIZO
    // Live_Raizo
    {
        .names = { "Live Raizo", "Live_Raizo" },
        .lines = FASTFETCH_DATATEXT_LOGO_LIVE_RAIZO,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LLIUREX
    // LliureX
    {
        .names = { "LliureX" },
        .lines = FASTFETCH_DATATEXT_LOGO_LLIUREX,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LMDE
    // LMDE
    {
        .names = { "LMDE" },
        .lines = FASTFETCH_DATATEXT_LOGO_LMDE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LOCOS
    // Loc-OS
    {
        .names = { "locos", "loc-os", "Loc-OS Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_LOCOS,
        .colors = {
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LUBUNTU
    // Lubuntu
    {
        .names = { "lubuntu" },
        .lines = FASTFETCH_DATATEXT_LOGO_LUBUNTU,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_LUNAR
    // Lunar
    {
        .names = { "Lunar" },
        .lines = FASTFETCH_DATATEXT_LOGO_LUNAR,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    // LAST
    {},
};
