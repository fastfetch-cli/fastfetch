#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo Z[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_ZERENE
    // Zerene
    {
        .names = { "Zerene, Zerene OS" },
        .lines = FASTFETCH_DATATEXT_LOGO_ZERENE,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ZORIN
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ZOS
    // Z/OS
    {
        .names = { "z/OS", "zos" },
        .lines = FASTFETCH_DATATEXT_LOGO_ZOS,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ZRAXYL
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
    #endif
    // LAST
    {},
};
