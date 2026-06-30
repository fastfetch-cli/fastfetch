#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo P[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_PACBSD
    // PacBSD
    {
        .names = { "PacBSD" },
        .lines = FASTFETCH_DATATEXT_LOGO_PACBSD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PANWAH
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PARABOLA
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PARABOLA_SMALL
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PARABOLA2_SMALL
    // Parabola2Small
    {
        .names = { "parabola2_small", "parabola-gnulinux2_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_PARABOLA2_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PARCH
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PARDUS
    // Pardus
    {
        .names = { "Pardus" },
        .lines = FASTFETCH_DATATEXT_LOGO_PARDUS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PARROT
    // Parrot
    {
        .names = { "Parrot" },
        .lines = FASTFETCH_DATATEXT_LOGO_PARROT,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PARSIX
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PCBSD
    // PCBSD
    {
        .names = { "PCBSD", "TrueOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_PCBSD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PCLINUXOS
    // PCLinuxOS
    {
        .names = { "PCLinuxOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_PCLINUXOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PEAROS
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PENGWIN
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PENTOO
    // Pentoo
    {
        .names = { "Pentoo" },
        .lines = FASTFETCH_DATATEXT_LOGO_PENTOO,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PEPPERMINT
    // Peppermint
    {
        .names = { "Peppermint" },
        .lines = FASTFETCH_DATATEXT_LOGO_PEPPERMINT,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PEROPESIS
    // Peropesis
    {
        .names = { "Peropesis", "Peropesis Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_PEROPESIS,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PHYOS
    // PhyOS
    {
        .names = { "PhyOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_PHYOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PIKAOS
    // PikaOS
    {
        .names = { "PikaOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_PIKAOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PISI
    // PisiLinux
    {
        .names = { "PisiLinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_PISI,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PNM_LINUX
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_POP
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_POP_SMALL
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PORTEUS
    // Porteus
    {
        .names = { "Porteus" },
        .lines = FASTFETCH_DATATEXT_LOGO_PORTEUS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_POSTMARKETOS
    // PostMarketOS
    {
        .names = { "PostMarketOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_POSTMARKETOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_POSTMARKETOS2
    // PostMarketOS2
    {
        .names = { "PostMarketOS2" },
        .lines = FASTFETCH_DATATEXT_LOGO_POSTMARKETOS2,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_POSTMARKETOS_SMALL
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PROXMOX
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PUFFOS
    // PuffOS
    {
        .names = { "PuffOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_PUFFOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PUPPY
    // Puppy
    {
        .names = { "Puppy" },
        .lines = FASTFETCH_DATATEXT_LOGO_PUPPY,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PUREOS
    // PureOS
    {
        .names = { "PureOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_PUREOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PUREOS_SMALL
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
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PRISMLINUX
    // PrismLinux
    {
        .names = { "PrismLinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_PRISMLINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_PRISMLINUX_SMALL
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
    #endif
    // LAST
    {},
};
