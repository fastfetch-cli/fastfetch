#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo R[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_RADIX
    // Radix
    {
        .names = { "Radix" },
        .lines = FASTFETCH_DATATEXT_LOGO_RADIX,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_RASPBIAN
    // Raspbian
    {
        .names = { "raspbian" },
        .lines = FASTFETCH_DATATEXT_LOGO_RASPBIAN,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_RASPBIAN_SMALL
    // RaspbianSmall
    {
        .names = { "raspbian_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_RASPBIAN_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_RAVYNOS
    // RavynOS
    {
        .names = { "RavynOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_RAVYNOS,
        .colors = {
            FF_COLOR_FG_256 "15",
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_REBORNOS
    // RebornOS
    {
        .names = { "RebornOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_REBORNOS,
        .colors = {
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_REBORNOS_SMALL
    // RebornSmall
    {
        .names = { "RebornOS_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_REBORNOS_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_REDCORE
    // RedCore
    {
        .names = { "RedCore" },
        .lines = FASTFETCH_DATATEXT_LOGO_REDCORE,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_RHEL
    // RedHatEnterpriseLinux
    {
        .names = { "rhel", "redhat" },
        .lines = FASTFETCH_DATATEXT_LOGO_RHEL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_RHEL_SMALL
    // RedHatEnterpriseLinux
    {
        .names = { "rhel_small", "redhat_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_RHEL_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_RHEL_OLD
    // RedHatEnterpriseLinux_old
    {
        .names = { "rhel_old", "redhat_old" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_RHEL_OLD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_REDOS
    // RedOS
    {
        .names = { "RedOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_REDOS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorTitle = FF_COLOR_FG_RED,
        .colorKeys = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_REDOS_SMALL
    // RedOS small
    {
        .names = { "RedOS_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_REDOS_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorTitle = FF_COLOR_FG_RED,
        .colorKeys = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_REDSTAR
    // RedstarOS
    {
        .names = { "redstar", "redstar-os", "redstaros" },
        .lines = FASTFETCH_DATATEXT_LOGO_REDSTAR,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_REDROSE
    // Redrose
    {
        .names = { "Redrose" },
        .lines = FASTFETCH_DATATEXT_LOGO_REDROSE,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_REFRACTA
    // Refracta
    {
        .names = { "Refracta" },
        .lines = FASTFETCH_DATATEXT_LOGO_REFRACTA,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_BLACK,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_REGATA
    // Regata
    {
        .names = { "Regata" },
        .lines = FASTFETCH_DATATEXT_LOGO_REGATA,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_REGOLITH
    // Regolith
    {
        .names = { "Regolith" },
        .lines = FASTFETCH_DATATEXT_LOGO_REGOLITH,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_RHAYMOS
    // RhaymOS
    {
        .names = { "RhaymOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_RHAYMOS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ROCKY
    // RockyLinux
    {
        .names = { "rocky" },
        .lines = FASTFETCH_DATATEXT_LOGO_ROCKY,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ROCKY_SMALL
    // RockyLinuxSmall
    {
        .names = { "rocky_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ROCKY_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ROSA
    // RosaLinux
    {
        .names = { "rosa", "rosa-linux", "rosalinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_ROSA,
        .colors = {
            FF_COLOR_FG_RGB "250;250;250",
            FF_COLOR_FG_RGB "100;165;225",
        },
        .colorKeys = FF_COLOR_FG_RGB "100;165;225",
        .colorTitle = FF_COLOR_FG_RGB "100;165;225",
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_RHINO
    // RhinoLinux
    {
        .names = { "Rhino Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_RHINO,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_LIGHT_BLUE,
            FF_COLOR_FG_LIGHT_MAGENTA,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_RENGEOS
    {
        .names = { "RengeOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_RENGEOS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_MAGENTA,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_RORKOS
    // RorkOS
    {
        .names = { "rorkos", "RorkOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_RORKOS,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_WHITE,
        .colorTitle = FF_COLOR_FG_WHITE,
    },
    #endif
    // LAST
    {},
};
