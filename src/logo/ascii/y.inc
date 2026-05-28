#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo Y[] = {
    // YiffOS
    {
        .names = { "YiffOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_YIFFOS,
        .colors = {
            FF_COLOR_FG_256 "93",
            FF_COLOR_FG_256 "92",
        },
    },
    // LAST
    {},
};
