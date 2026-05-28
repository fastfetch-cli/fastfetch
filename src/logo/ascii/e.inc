#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo E[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_ELBRUS
    // Elbrus
    {
        .names = { "elbrus" },
        .lines = FASTFETCH_DATATEXT_LOGO_ELBRUS,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ELEMENTARY
    // Elementary
    {
        .names = { "Elementary" },
        .lines = FASTFETCH_DATATEXT_LOGO_ELEMENTARY,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ELEMENTARY_SMALL
    // ElementarySmall
    {
        .names = { "Elementary_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ELEMENTARY_SMALL,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ELIVE
    // Elive
    {
        .names = { "Elive" },
        .lines = FASTFETCH_DATATEXT_LOGO_ELIVE,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_CYAN,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_EMMABUNTUS
    // Emmabuntüs
    {
        .names = { "Emmabuntus" },
        .lines = FASTFETCH_DATATEXT_LOGO_EMMABUNTUS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_YELLOW,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_EMPEROROS
    // EmperorOS
    {
        .names = { "Emperor" },
        .lines = FASTFETCH_DATATEXT_LOGO_EMPEROROS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_DEFAULT,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ENOS
    // EN-OS
    {
        .names = { "ENOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_ENOS,
        .colors = {
            FF_COLOR_FG_LIGHT_BLUE,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLUE,
        .colorTitle = FF_COLOR_FG_LIGHT_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ENCRYPTOS
    // EncryptOS
    {
        .names = { "EncryptOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_ENCRYPTOS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ENDEAVOUROS
    // EndeavourOS
    {
        .names = { "EndeavourOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_ENDEAVOUROS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ENDEAVOUROS_SMALL
    // EndeavourOSSmall
    {
        .names = { "EndeavourOS_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ENDEAVOUROS_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLUE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ENDLESS
    // Endless
    {
        .names = { "Endless" },
        .lines = FASTFETCH_DATATEXT_LOGO_ENDLESS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ENSO
    // Enso
    {
        .names = { "Enso" },
        .lines = FASTFETCH_DATATEXT_LOGO_ENSO,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ESHANIZEDOS
    // EshanizedOS
    {
        .names = { "EshanizedOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_ESHANIZEDOS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_EUROLINUX
    // EuroLinux
    {
        .names = { "EuroLinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_EUROLINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_EVOLUTIONOS
    // EvolutionOS
    {
        .names = { "EvolutionOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_EVOLUTIONOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_EVOLUTIONOS_SMALL
    // EvolutionOSSmall
    {
        .names = { "EvolutionOS_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_EVOLUTIONOS_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_EVOLUTIONOS_OLD
    // EvolutionOS_old
    {
        .names = { "EvolutionOS_old" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_EVOLUTIONOS_OLD,
        .colors = {
            FF_COLOR_FG_LIGHT_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_EWEOS
    // eweOS
    {
        .names = { "eweOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_EWEOS,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_YELLOW,
            FF_COLOR_FG_LIGHT_RED,
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_RED,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_EXHERBO
    // Exherbo
    {
        .names = { "Exherbo" },
        .lines = FASTFETCH_DATATEXT_LOGO_EXHERBO,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_EXODIA_PREDATOR
    // ExodiaOS
    {
        .names = { "Exodia" },
        .lines = FASTFETCH_DATATEXT_LOGO_EXODIA_PREDATOR,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    #endif
    // LAST
    {},
};
