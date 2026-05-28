#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo I[] = {
    // Iglunix
    {
        .names = { "Iglunix", "Iglu" },
        .lines = FASTFETCH_DATATEXT_LOGO_IGLUNIX,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // InstantOS
    {
        .names = { "InstantOS" },
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
        .names = { "Interix" },
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
        .names = { "IRIX" },
        .lines = FASTFETCH_DATATEXT_LOGO_IRIX,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Ironclad
    {
        .names = { "Ironclad" },
        .lines = FASTFETCH_DATATEXT_LOGO_IRONCLAD,
        .colors = {
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // Itc
    {
        .names = { "Itc" },
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
