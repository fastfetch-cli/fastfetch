#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo Q[] = {
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
    // Q4OS
    {
        .names = { "Q4OS" },
        .lines = FASTFETCH_DATATEXT_LOGO_Q4OS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_RED,
        },
    },
    // Qubes
    {
        .names = { "Qubes" },
        .lines = FASTFETCH_DATATEXT_LOGO_QUBES,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
        },
    },
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
    // Quibian
    {
        .names = { "Quibian" },
        .lines = FASTFETCH_DATATEXT_LOGO_QUIBIAN,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    // Quirinux
    {
        .names = { "Quirinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_QUIRINUX,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_MAGENTA,
        },
    },
    // LAST
    {},
};
