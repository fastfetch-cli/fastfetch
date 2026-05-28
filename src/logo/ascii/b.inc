#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo B[] = {
    // Bedrock
    {
        .names = { "bedrock" },
        .lines = FASTFETCH_DATATEXT_LOGO_BEDROCK,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK, // grey
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK, // grey
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // BedrockSmall
    {
        .names = { "bedrock_small" },
        .lines = FASTFETCH_DATATEXT_LOGO_BEDROCK_SMALL,
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK, // grey
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK, // grey
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // BigLinux
    {
        .names = { "BigLinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_BIGLINUX,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Bitrig
    {
        .names = { "Bitrig" },
        .lines = FASTFETCH_DATATEXT_LOGO_BITRIG,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // BlackArch
    {
        .names = { "Blackarch" },
        .lines = FASTFETCH_DATATEXT_LOGO_BLACKARCH,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_LIGHT_RED,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // BlackMesa
    {
        .names = { "BlackMesa", "black-mesa" },
        .lines = FASTFETCH_DATATEXT_LOGO_BLACKMESA,
        .colors = {
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_BLACK,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // BlackPanther
    {
        .names = { "BlackPanther" },
        .lines = FASTFETCH_DATATEXT_LOGO_BLACKPANTHER,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_LIGHT_BLUE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // BLAG
    {
        .names = { "BLAG" },
        .lines = FASTFETCH_DATATEXT_LOGO_BLAG,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // BlankOn
    {
        .names = { "BlankOn" },
        .lines = FASTFETCH_DATATEXT_LOGO_BLANKON,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // BlueLight
    {
        .names = { "BlueLight" },
        .lines = FASTFETCH_DATATEXT_LOGO_BLUELIGHT,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Bodhi
    {
        .names = { "Bodhi" },
        .lines = FASTFETCH_DATATEXT_LOGO_BODHI,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_YELLOW,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Bonsai
    {
        .names = { "Bonsai" },
        .lines = FASTFETCH_DATATEXT_LOGO_BONSAI,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // BredOS
    {
        .names = { "Bredos" },
        .lines = FASTFETCH_DATATEXT_LOGO_BREDOS,
        .colors = {
            FF_COLOR_FG_RGB "198;151;66", // grey
        },
        .colorKeys = FF_COLOR_FG_RGB "198;151;66",
        .colorTitle = FF_COLOR_FG_RGB "198;151;66",
    },
    // BSD
    {
        .names = { "BSD" },
        .lines = FASTFETCH_DATATEXT_LOGO_BSD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // BunsenLabs
    {
        .names = { "BunsenLabs" },
        .lines = FASTFETCH_DATATEXT_LOGO_BUNSENLABS,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // LAST
    {},
};
