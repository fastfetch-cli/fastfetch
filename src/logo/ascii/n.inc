#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo N[] = {
    // Namib
    {
        .names = { "Namib" },
        .lines = FASTFETCH_DATATEXT_LOGO_NAMIB,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // NebiOS
    {
        .names = { "NebiOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_NEBIOS,
        .colors = {
            FF_COLOR_FG_RGB "255;95;95",
            FF_COLOR_FG_RGB "252;146;84",
            FF_COLOR_FG_RGB "248;196;72",
        },
    },
    // Nekos
    {
        .names = { "Nekos" },
        .lines = FASTFETCH_DATATEXT_LOGO_NEKOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
    },
    // Neptune
    {
        .names = { "Neptune" },
        .lines = FASTFETCH_DATATEXT_LOGO_NEPTUNE,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_WHITE,
        },
    },
    // NetRunner
    {
        .names = { "NetRunner" },
        .lines = FASTFETCH_DATATEXT_LOGO_NETRUNNER,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // NexaLinux
    {
        .names = { "nexalinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_NEXALINUX,
        .colors = {
            FF_COLOR_FG_LIGHT_BLUE,
            FF_COLOR_FG_LIGHT_BLUE,
        },
    },
    // Nitrux
    {
        .names = { "Nitrux" },
        .lines = FASTFETCH_DATATEXT_LOGO_NITRUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // NixOS
    {
        .names = { "NixOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // NixOSSmall
    {
        .names = { "NixOS_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // NixOSOld
    {
        .names = { "nixos_old" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS_OLD,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // NixOsOldSmall
    {
        .names = { "nixos_old_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_NIXOS_OLD_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // NetBSD
    {
        .names = { "NetBSD" },
        .lines = FASTFETCH_DATATEXT_LOGO_NETBSD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // NetBSD2
    {
        .names = { "NetBSD2" },
        .lines = FASTFETCH_DATATEXT_LOGO_NETBSD2,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // NetBSD Small
    {
        .names = { "NetBSD_small" },
        .lines = FASTFETCH_DATATEXT_LOGO_NETBSD_SMALL,
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Nobara
    {
        .names = { "nobara" },
        .lines = FASTFETCH_DATATEXT_LOGO_NOBARA,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // NomadBSD
    {
        .names = { "nomadbsd" },
        .lines = FASTFETCH_DATATEXT_LOGO_NOMADBSD,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // NurOS
    {
        .names = { "NurOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_NUROS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Nurunner
    {
        .names = { "Nurunner" },
        .lines = FASTFETCH_DATATEXT_LOGO_NURUNNER,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // NuTyX
    {
        .names = { "NuTyX" },
        .lines = FASTFETCH_DATATEXT_LOGO_NUTYX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_RED,
        },
    },
    // NetHydra
    {
        .names = { "NetHydra" },
        .lines = FASTFETCH_DATATEXT_LOGO_NETHYDRA,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_DEFAULT,
        },
    },
    // LAST
    {},
};
