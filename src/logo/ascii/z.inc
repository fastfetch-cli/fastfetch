#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo Z[] = {
    // Zorin
    {
        .names = { "Zorin" },
        .lines = FASTFETCH_DATATEXT_LOGO_ZORIN,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Z/OS
    {
        .names = { "z/OS", "zos" },
        .lines = FASTFETCH_DATATEXT_LOGO_ZOS,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
    },
    // Zraxyl
    {
        .names = { "Zraxyl" },
        .lines = FASTFETCH_DATATEXT_LOGO_ZRAXYL,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // LAST
    {},
};
