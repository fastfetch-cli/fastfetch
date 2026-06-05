#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo D[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_DAHLIA
    // DahliaOS
    {
        .names = { "dahliaOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_DAHLIA,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_DARKOS
    // DarkOS
    {
        .names = { "DarkOS" },
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_DEBIAN
    // Debian
    {
        .names = { "Debian" },
        .lines = FASTFETCH_DATATEXT_LOGO_DEBIAN,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_DEBIAN_SMALL
    // DebianSmall
    {
        .names = { "Debian_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_DEBIAN_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_DEEPIN
    // Deepin
    {
        .names = { "Deepin" },
        .lines = FASTFETCH_DATATEXT_LOGO_DEEPIN,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_DESAOS
    // DesaOS
    {
        .names = { "DesaOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_DESAOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_DEVUAN
    // Devuan
    {
        .names = { "Devuan" },
        .lines = FASTFETCH_DATATEXT_LOGO_DEVUAN,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_DEVUAN_SMALL
    // DevuanSmall
    {
        .names = { "Devuan_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_DEVUAN_SMALL,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_DIETPI
    // DietPi
    {
        .names = { "DietPi" },
        .lines = FASTFETCH_DATATEXT_LOGO_DIETPI,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_DRACOS
    // DracOS
    {
        .names = { "DracOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_DRACOS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_DRAGONFLY
    // DragonFly
    {
        .names = { "DragonFly" },
        .lines = FASTFETCH_DATATEXT_LOGO_DRAGONFLY,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_DRAGONFLY_SMALL
    // DragonFlySmall
    {
        .names = { "DragonFly_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_DRAGONFLY_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_DRAGONFLY_OLD
    // DragonFlyOld
    {
        .names = { "DragonFly_old" },
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_DRAUGER
    // DraugerOS
    {
        .names = { "DraugerOS", "Drauger" },
        .lines = FASTFETCH_DATATEXT_LOGO_DRAUGER,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_DROIDIAN
    // Droidian
    {
        .names = { "Droidian" },
        .lines = FASTFETCH_DATATEXT_LOGO_DROIDIAN,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_LIGHT_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_LIGHT_GREEN,
    },
    #endif
    // LAST
    {},
};
