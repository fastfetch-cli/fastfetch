#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo P[] = {
    // PacBSD
    {
        .names = { "PacBSD" },
        .lines = FASTFETCH_DATATEXT_LOGO_PACBSD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // Panwah
    {
        .names = { "Panwah" },
        .lines = FASTFETCH_DATATEXT_LOGO_PANWAH,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_BLACK,
        },
    },
    // Parabola
    {
        .names = { "parabola", "parabola-gnulinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_PARABOLA,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // ParabolaSmall
    {
        .names = { "parabola_small", "parabola-gnulinux_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_PARABOLA_SMALL,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    // Parch
    {
        .names = { "Parch" },
        .lines = FASTFETCH_DATATEXT_LOGO_PARCH,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
    },
    // Pardus
    {
        .names = { "Pardus" },
        .lines = FASTFETCH_DATATEXT_LOGO_PARDUS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // Parrot
    {
        .names = { "Parrot" },
        .lines = FASTFETCH_DATATEXT_LOGO_PARROT,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // Parsix
    {
        .names = { "Parsix" },
        .lines = FASTFETCH_DATATEXT_LOGO_PARSIX,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
    },
    // PCBSD
    {
        .names = { "PCBSD", "TrueOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_PCBSD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // PCLinuxOS
    {
        .names = { "PCLinuxOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_PCLINUXOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // PearOS
    {
        .names = { "PearOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_PEAROS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLUE,
        },
    },
    // Pengwin
    {
        .names = { "Pengwin" },
        .lines = FASTFETCH_DATATEXT_LOGO_PENGWIN,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_LIGHT_MAGENTA,
            FF_COLOR_FG_MAGENTA,
        },
    },
    // Pentoo
    {
        .names = { "Pentoo" },
        .lines = FASTFETCH_DATATEXT_LOGO_PENTOO,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
    },
    // Peppermint
    {
        .names = { "Peppermint" },
        .lines = FASTFETCH_DATATEXT_LOGO_PEPPERMINT,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // Peropesis
    {
        .names = { "Peropesis", "Peropesis Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_PEROPESIS,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
    },
    // PhyOS
    {
        .names = { "PhyOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_PHYOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // PikaOS
    {
        .names = { "PikaOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_PIKAOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
    },
    // PisiLinux
    {
        .names = { "PisiLinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_PISI,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // PNMLinux
    {
        .names = { "PNM Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_PNM_LINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_256 "202",
        },
    },
    // Pop
    {
        .names = { "pop", "popos" },
        .lines = FASTFETCH_DATATEXT_LOGO_POP,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // PopSmall
    {
        .names = { "pop_small", "popos_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_POP_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    // Porteus
    {
        .names = { "Porteus" },
        .lines = FASTFETCH_DATATEXT_LOGO_PORTEUS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // PostMarketOS
    {
        .names = { "PostMarketOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_POSTMARKETOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    // PostMarketOSSmall
    {
        .names = { "PostMarketOS_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_POSTMARKETOS_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    // Proxmox
    {
        .names = { "Proxmox", "pve" },
        .lines = FASTFETCH_DATATEXT_LOGO_PROXMOX,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_256 "202",
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_256 "202",
    },
    // PuffOS
    {
        .names = { "PuffOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_PUFFOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    // Puppy
    {
        .names = { "Puppy" },
        .lines = FASTFETCH_DATATEXT_LOGO_PUPPY,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // PureOS
    {
        .names = { "PureOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_PUREOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    // PureOSSmall
    {
        .names = { "PureOS_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_PUREOS_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    // PrismLinux
    {
        .names = { "PrismLinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_PRISMLINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
        },
    },
    // PrismLinuxSmall
    {
        .names = { "PrismLinux_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_PRISMLINUX_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
        },
    },
    // LAST
    {},
};
