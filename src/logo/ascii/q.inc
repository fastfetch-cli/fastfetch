#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo Q[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_QTS
    // QTS
    {
        .names = { "qts" },
        .lines = FASTFETCH_DATATEXT_LOGO_QTS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_QUASAR
    // Quasar
    {
        .names = { "Quasar" },
        .lines = FASTFETCH_DATATEXT_LOGO_QUASAR,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_Q4OS
    // Q4OS
    {
        .names = { "Q4OS" },
        .lines = FASTFETCH_DATATEXT_LOGO_Q4OS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_RED,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_QUBES
    // Qubes
    {
        .names = { "Qubes" },
        .lines = FASTFETCH_DATATEXT_LOGO_QUBES,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_QUBES_SMALL
    // Qubes
    {
        .names = { "Qubes_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_QUBES_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_QUBYT
    // Qubyt
    {
        .names = { "Qubyt" },
        .lines = FASTFETCH_DATATEXT_LOGO_QUBYT,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLACK,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_QUIBIAN
    // Quibian
    {
        .names = { "Quibian" },
        .lines = FASTFETCH_DATATEXT_LOGO_QUIBIAN,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_QUIRINUX
    // Quirinux
    {
        .names = { "Quirinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_QUIRINUX,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_MAGENTA,
        },
    },
    #endif
    // LAST
    {},
};
