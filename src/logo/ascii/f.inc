#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo F[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_FASTFETCH
    // Fastfetch
    {
        .names = { "Fastfetch", "FF" },
        .lines = FASTFETCH_DATATEXT_LOGO_FASTFETCH,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_DEFAULT,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FEDORA
    // Fedora
    {
        .names = { "Fedora" },
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ASAHI
    // FedoraAsahiRemix
    {
        .names = { "fedora-asahi-remix" },
        .lines = FASTFETCH_DATATEXT_LOGO_ASAHI,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FEDORA_SMALL
    // FedoraSmall
    {
        .names = { "Fedora_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FEDORA2_SMALL
    {
        .names = { "Fedora2_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA2_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FEDORA_OLD
    // FedoraOld
    {
        .names = { "Fedora_old" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_OLD,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FEDORA_SILVERBLUE
    // FedoraSilverblue
    {
        .names = { "Fedora-Silverblue" },
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_SILVERBLUE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FEDORA_KINOITE
    // FedoraKinoite
    {
        .names = { "Fedora-Kinoite" },
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_KINOITE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FEDORA_SERICEA
    // FedoraSericea
    {
        .names = { "Fedora-Sericea" },
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_SERICEA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FEDORA_COREOS
    // FedoraCoreOS
    {
        .names = { "Fedora-CoreOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_COREOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FEMBOYOS
    // FemboyOS
    {
        .names = { "FemboyOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_FEMBOYOS,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FEREN
    // Feren
    {
        .names = { "Feren" },
        .lines = FASTFETCH_DATATEXT_LOGO_FEREN,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FLATCAR
    // Flatcar
    {
        .names = { "Flatcar" },
        .lines = FASTFETCH_DATATEXT_LOGO_FLATCAR,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FILOTIMO
    // Filotimo
    {
        .names = { "Filotimo" },
        .lines = FASTFETCH_DATATEXT_LOGO_FILOTIMO,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FINNIX
    // Finnix
    {
        .names = { "Finnix" },
        .lines = FASTFETCH_DATATEXT_LOGO_FINNIX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FLOFLIS
    // Floflis
    {
        .names = { "Floflis" },
        .lines = FASTFETCH_DATATEXT_LOGO_FLOFLIS,
        .colors = {
            FF_COLOR_FG_LIGHT_CYAN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FREEBSD
    // FreeBSD
    {
        .names = { "Freebsd" },
        .lines = FASTFETCH_DATATEXT_LOGO_FREEBSD,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FREEBSD_SMALL
    // FreeBSDSmall
    {
        .names = { "freebsd_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_FREEBSD_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FREEMINT
    // FreeMiNT
    {
        .names = { "FreeMiNT" },
        .lines = FASTFETCH_DATATEXT_LOGO_FREEMINT,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FRUGALWARE
    // Frugalware
    {
        .names = { "Frugalware", "frugalware-linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_FRUGALWARE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FUNTOO
    // Funtoo
    {
        .names = { "Funtoo" },
        .lines = FASTFETCH_DATATEXT_LOGO_FUNTOO,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FURRETO
    // Furreto
    {
        .names = { "Furreto" },
        .lines = FASTFETCH_DATATEXT_LOGO_FURRETO,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    // LAST
    {},
};
