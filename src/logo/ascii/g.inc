#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo G[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_GALLIUMOS
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GARUDA
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GARUDA_DRAGON
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GARUDA_SMALL
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GENTOO
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GENTOO_SMALL
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GHOSTBSD
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GHOSTFREAK
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GLAUCUS
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GNEWSENSE
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GNOME
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GNU
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GOBOLINUX
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GOLDENDOGLINUX
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GRAPHENEOS
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GROMBYANG
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GUIX
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GUIX_SMALL
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_GXDE
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
    #endif
    // LAST
    {},
};
