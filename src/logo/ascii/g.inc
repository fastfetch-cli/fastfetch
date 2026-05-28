#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo G[] = {
    // GalliumOS
    {
        .names = { "GalliumOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_GALLIUMOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Garuda
    {
        .names = { "Garuda", "garuda-linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_GARUDA,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // GarudaDragon
    {
        .names = { "GarudaDragon", "garuda-dragon" },
        .lines = FASTFETCH_DATATEXT_LOGO_GARUDA_DRAGON,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // GarudaSmall
    {
        .names = { "Garuda_small", "garuda-linux_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_GARUDA_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Gentoo
    {
        .names = { "Gentoo" },
        .lines = FASTFETCH_DATATEXT_LOGO_GENTOO,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // GentooSmall
    {
        .names = { "Gentoo_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_GENTOO_SMALL,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // GhostBSD
    {
        .names = { "GhostBSD" },
        .lines = FASTFETCH_DATATEXT_LOGO_GHOSTBSD,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // GhostFreak
    {
        .names = { "GhostFreak" },
        .lines = FASTFETCH_DATATEXT_LOGO_GHOSTFREAK,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Glaucus
    {
        .names = { "Glaucus" },
        .lines = FASTFETCH_DATATEXT_LOGO_GLAUCUS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // GNewSense
    {
        .names = { "gNewSense" },
        .lines = FASTFETCH_DATATEXT_LOGO_GNEWSENSE,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // GNOME OS
    {
        .names = { "GNOME OS" }, // matches "NAME"
        .lines = FASTFETCH_DATATEXT_LOGO_GNOME,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // GNU
    {
        .names = { "GNU" },
        .lines = FASTFETCH_DATATEXT_LOGO_GNU,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // GoboLinux
    {
        .names = { "GoboLinux", "Gobo" },
        .lines = FASTFETCH_DATATEXT_LOGO_GOBOLINUX,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // GoldenDogLinux
    {
        .names = { "GoldenDog Linux", "GDL", "goldendoglinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_GOLDENDOGLINUX,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // GrapheneOS
    {
        .names = { "GrapheneOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_GRAPHENEOS,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Grombyang
    {
        .names = { "Grombyang" },
        .lines = FASTFETCH_DATATEXT_LOGO_GROMBYANG,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    // Guix
    {
        .names = { "Guix" },
        .lines = FASTFETCH_DATATEXT_LOGO_GUIX,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // GuixSmall
    {
        .names = { "Guix_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_GUIX_SMALL,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // GXDE
    {
        .names = { "GXDE" },
        .lines = FASTFETCH_DATATEXT_LOGO_GXDE,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // LAST
    {},
};
