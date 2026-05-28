#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo F[] = {
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
    // Filotimo
    {
        .names = { "filotimo" },
        .lines = FASTFETCH_DATATEXT_LOGO_FILOTIMO,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
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
    // LAST
    {},
};
