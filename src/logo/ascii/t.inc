#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo T[] = {
    // T2
    {
        .names = { "T2", "T2 SDE", "T2/Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_T2,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE,
        },
    },
    // T2Small
    {
        .names = { "T2_small", "T2 SDE_small", "T2/Linux_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_T2_SMALL,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
    },
    // Tails
    {
        .names = { "Tails" },
        .lines = FASTFETCH_DATATEXT_LOGO_TAILS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Tatra
    {
        .names = { "Tatra" },
        .lines = FASTFETCH_DATATEXT_LOGO_TATRA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_GREEN,
        },
    },
    // TeArch
    {
        .names = { "TeArch" },
        .lines = FASTFETCH_DATATEXT_LOGO_TEARCH,
        .colors = {
            FF_COLOR_FG_256 "39",
            FF_COLOR_FG_WHITE,
        },
    },
    // TempleOS
    {
        .names = { "TempleOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_TEMPLEOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_CYAN,
        },
    },
    // TileOS
    {
        .names = { "TileOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_TILEOS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_GREEN,
        },
    },
    // Torizon OS
    {
        .names = { "Torizon OS", "TorizonCore" },
        .lines = FASTFETCH_DATATEXT_LOGO_TORIZONCORE,
        .colors = {
            FF_COLOR_FG_LIGHT_WHITE,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE,
        },
    },
    // Trisquel
    {
        .names = { "Trisquel" },
        .lines = FASTFETCH_DATATEXT_LOGO_TRISQUEL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // TrueNAS Scale
    {
        .names = { "TrueNAS-Scale" },
        .lines = FASTFETCH_DATATEXT_LOGO_TRUENAS_SCALE,
        .colors = {
            FF_COLOR_FG_256 "39",
            FF_COLOR_FG_256 "32",
            FF_COLOR_FG_256 "248",
        },
        .colorKeys = FF_COLOR_FG_256 "248",
        .colorTitle = FF_COLOR_FG_256 "32",
    },
    // TuxedoOS
    {
        .names = { "Tuxedo OS", "tuxedo" },
        .lines = FASTFETCH_DATATEXT_LOGO_TUXEDO_OS,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
    },
    // Twister
    {
        .names = { "Twister" },
        .lines = FASTFETCH_DATATEXT_LOGO_TWISTER,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
    },
    // LAST
    {},
};
