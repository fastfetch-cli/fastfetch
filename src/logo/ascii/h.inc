#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo H[] = {
    // Haiku
    {
        .names = { "Haiku" },
        .lines = FASTFETCH_DATATEXT_LOGO_HAIKU,
        .colors = {
            FF_COLOR_FG_DEFAULT,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // Haiku2
    {
        .names = { "Haiku2" },
        .lines = FASTFETCH_DATATEXT_LOGO_HAIKU2,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // HaikuSmall
    {
        .names = { "Haiku_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_HAIKU_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    // HamoniKR
    {
        .names = { "HamoniKR" },
        .lines = FASTFETCH_DATATEXT_LOGO_HAMONIKR,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_256 "99",
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // HarDClanZ
    {
        .names = { "HarDClanZ" },
        .lines = FASTFETCH_DATATEXT_LOGO_HARDCLANZ,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // HardenedBSD
    {
        .names = { "HardenedBSD" },
        .lines = FASTFETCH_DATATEXT_LOGO_FREEBSD,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // HarmonyOS
    {
        .names = { "HarmonyOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_HARMONYOS,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Hash
    {
        .names = { "Hash" },
        .lines = FASTFETCH_DATATEXT_LOGO_HASH,
        .colors = {
            FF_COLOR_FG_256 "123",
            FF_COLOR_FG_256 "123",
        },
    },
    // HeliumOS
    {
        .names = { "HeliumOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_HELIUMOS,
        .colors = {
            FF_COLOR_FG_256 "81",
        },
        .colorKeys = FF_COLOR_FG_256 "81",
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Huawei Cloud EulerOS
    {
        .names = { "Huawei Cloud EulerOS", "hce" },
        .lines = FASTFETCH_DATATEXT_LOGO_HCE,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Huayra
    {
        .names = { "Huayra" },
        .lines = FASTFETCH_DATATEXT_LOGO_HUAYRA,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Hybrid
    {
        .names = { "Hybrid" },
        .lines = FASTFETCH_DATATEXT_LOGO_HYBRID,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_LIGHT_BLUE,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // HydroOS
    {
        .names = { "HydroOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_HYDROOS,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // HyprOS
    {
        .names = { "hypros" },
        .lines = FASTFETCH_DATATEXT_LOGO_HYPROS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
        },
    },
    // Hyperbola
    {
        .names = { "Hyperbola" },
        .lines = FASTFETCH_DATATEXT_LOGO_HYPERBOLA,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // HyperbolaSmall
    {
        .names = { "Hyperbola_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_HYPERBOLA_SMALL,
        .colors = {
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // LAST
    {},
};
