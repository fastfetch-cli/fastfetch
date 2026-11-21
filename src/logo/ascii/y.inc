#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo Y[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_YANDEXSTATION
    // YandexStation
    {
        .names = { "YandexStation" },
        .lines = FASTFETCH_DATATEXT_LOGO_YANDEXSTATION,
        .colors = {
            FF_COLOR_MODE_BOLD FF_COLOR_FG_RGB "122;85;255",
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_YIFFOS
    // YiffOS
    {
        .names = { "YiffOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_YIFFOS,
        .colors = {
            FF_COLOR_FG_256 "93",
            FF_COLOR_FG_256 "92",
        },
    },
    #endif
    // LAST
    {},
};
