#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo O[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_OBARUN
    // Obarun
    {
        .names = { "Obarun" },
        .lines = FASTFETCH_DATATEXT_LOGO_OBARUN,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OBREVENGE
    // OBRevenge
    {
        .names = { "OBRevenge" },
        .lines = FASTFETCH_DATATEXT_LOGO_OBREVENGE,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OBSIDIANOS
    // ObsidianOS
    {
        .names = { "ObsidianOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_OBSIDIANOS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_LIGHT_BLUE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OMNIOS
    // OmniOS
    {
        .names = { "OmniOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_OMNIOS,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_LIGHT_BLACK,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENKYLIN
    // OpenKylin
    {
        .names = { "openkylin", "open-kylin" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENKYLIN,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENBSD
    // OpenBSD
    {
        .names = { "openbsd" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENBSD,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENBSD_SMALL
    // OpenBSDSmall
    {
        .names = { "openbsd_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_OPENBSD_SMALL,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENEULER
    // OpenEuler
    {
        .names = { "OpenEuler" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENEULER,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENINDIANA
    // OpenIndiana
    {
        .names = { "OpenIndiana" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENINDIANA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_DEFAULT,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENMAMBA
    // OpenMamba
    {
        .names = { "OpenMamba" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENMAMBA,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_GREEN,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENRUYI
    // openRuyi
    {
        .names = { "openRuyi" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENRUYI,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_LIGHT_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENSTAGE
    // OpenStage
    {
        .names = { "OpenStage" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSTAGE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENSUSE
    // OpenSuse
    {
        .names = { "opensuse" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENSUSE_SMALL
    // OpenSuseSmall
    {
        .names = { "opensuse_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENSUSE_MICROOS
    // openSuseMicroOS
    {
        .names = { "opensuse-microos" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_MICROOS,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENSUSE_LEAP
    // OpenSuseLeap
    {
        .names = { "opensuse-leap" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_LEAP,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENSUSE_LEAP_OLD
    // OpenSuseLeapOld
    {
        .names = { "opensuse-leap_old" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_LEAP_OLD,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED
    // OpenSuseTumbleweed
    {
        .names = { "opensuse-tumbleweed" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED_SMALL
    // OpenSuseTumbleweedSmall
    {
        .names = { "opensuse-tumbleweed_small" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED_SMALL,
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED_OLD
    // OpenSuseTumbleweedOld
    {
        .names = { "opensuse-tumbleweed_old" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED_OLD,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED2
    // OpenSuseTumbleweed2
    {
        .names = { "opensuse-tumbleweed2" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_TUMBLEWEED2,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENSUSE_SLOWROLL
    // openSUSESlowroll
    {
        .names = { "opensuse-slowroll", "opensuse-tumbleweed-slowroll" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_SLOWROLL,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENMANDRIVA
    // OpenMandriva
    {
        .names = { "openmandriva", "open-mandriva", "open_mandriva", "openmandriva lx" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENMANDRIVA,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENWRT
    // OpenWrt
    {
        .names = { "openwrt" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENWRT,
        .colors = {
            FF_COLOR_FG_DEFAULT,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENWRT_SMALL
    // OpenWrtSmall
    {
        .names = { "openwrt_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_OPENWRT_SMALL,
        .colors = {
            FF_COLOR_FG_DEFAULT,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPENWRT_OLD
    // OpenWrtOld
    {
        .names = { "openwrt_old" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENWRT_OLD,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OPNSENSE
    // OPNsense
    {
        .names = { "OPNsense" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPNSENSE,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_256 "202",
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ORACLE
    // Oracle
    {
        .names = { "oracle", "oracle linux", "oracle linux server" },
        .lines = FASTFETCH_DATATEXT_LOGO_ORACLE,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ORCHID
    // Orchid
    {
        .names = { "orchid" },
        .lines = FASTFETCH_DATATEXT_LOGO_ORCHID,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ORCHID_SMALL
    // OrchidSmall
    {
        .names = { "orchid_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ORCHID_SMALL,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OREON
    // Oreon
    {
        .names = { "Oreon" },
        .lines = FASTFETCH_DATATEXT_LOGO_OREON,
        .colors = {
            FF_COLOR_FG_DEFAULT,
            FF_COLOR_FG_DEFAULT,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ORIGAMI
    // Origami
    {
        .names = { "Origami" },
        .lines = FASTFETCH_DATATEXT_LOGO_ORIGAMI,
        .colors = {
            FF_COLOR_FG_RGB "185;210;235",   // $1 sail — light sky blue
            FF_COLOR_FG_RGB "40;90;160",     // $2 hull — deeper navy
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ORIGAMI_SMALL
    // OrigamiSmall
    {
        .names = { "Origami_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_ORIGAMI_SMALL,
        .colors = {
            FF_COLOR_FG_RGB "185;210;235",   // $1 sail — light sky blue
            FF_COLOR_FG_RGB "40;90;160",     // $2 hull — deeper navy
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OS2WARP
    // OS/2 Warp
    {
        .names = { "OS2Warp" },
        .lines = FASTFETCH_DATATEXT_LOGO_OS2WARP,
        .colors = {
            FF_COLOR_FG_LIGHT_WHITE,
            FF_COLOR_FG_LIGHT_BLUE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OS_ELBRUS
    // OS_Elbrus
    {
        .names = { "OS Elbrus" },
        .lines = FASTFETCH_DATATEXT_LOGO_OS_ELBRUS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_OSMC
    // OSMC
    {
        .names = { "OSMC", "Open Source Media Center" },
        .lines = FASTFETCH_DATATEXT_LOGO_OSMC,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_MACOS
    // OSX
    {
        .names = { "OSX" },
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_LIGHT_RED,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_MACOS_SMALL
    // OSXSmall
    {
        .names = { "OSX_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_MACOS_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    // LAST
    {},
};
