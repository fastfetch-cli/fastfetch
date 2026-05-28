#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo V[] = {
    // Valhalla
    {
        .names = { "Valhalla", "valhallaos", "valhalla-linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_VALHALLA,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Vanilla
    {
        .names = { "vanilla" },
        .lines = FASTFETCH_DATATEXT_LOGO_VANILLA,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // Vanilla2
    {
        .names = { "vanilla2" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_VANILLA2,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // VanillaSmall
    {
        .names = { "vanilla_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_VANILLA_SMALL,
        .colors = {
            FF_COLOR_FG_LIGHT_YELLOW,
            FF_COLOR_FG_YELLOW,
        },
    },
    // Venom
    {
        .names = { "Venom" },
        .lines = FASTFETCH_DATATEXT_LOGO_VENOM,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_BLUE,
        },
    },
    // VenomSmall
    {
        .names = { "Venom_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_VENOM_SMALL,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_BLUE,
        },
    },
    // VincentOS
    {
        .names = { "VincentOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_VINCENTOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_DEFAULT,
        },
    },
    // Vnux
    {
        .names = { "Vnux" },
        .lines = FASTFETCH_DATATEXT_LOGO_VNUX,
        .colors = {
            FF_COLOR_FG_256 "11",
            FF_COLOR_FG_256 "8",
            FF_COLOR_FG_256 "15",
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // Vzlinux
    {
        .names = { "Vzlinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_VZLINUX,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
    },
    // Void
    {
        .names = { "void" },
        .lines = FASTFETCH_DATATEXT_LOGO_VOID,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // VoidSmall
    {
        .names = { "void_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_VOID_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Void2
    {
        .names = { "void2" },
        .lines = FASTFETCH_DATATEXT_LOGO_VOID2,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_DEFAULT,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Void2Small
    {
        .names = { "void2_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_VOID2_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // LAST
    {},
};
