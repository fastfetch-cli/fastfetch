#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo J[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_JANUSLINUX
    // Januslinux
    {
        .names = { "januslinux", "janus" },
        .lines = FASTFETCH_DATATEXT_LOGO_JANUSLINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    #endif
    // LAST
    {},
};
