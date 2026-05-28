#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo O[] = {
    // Obarun
    {
        .names = { "Obarun" },
        .lines = FASTFETCH_DATATEXT_LOGO_OBARUN,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // OBRevenge
    {
        .names = { "OBRevenge" },
        .lines = FASTFETCH_DATATEXT_LOGO_OBREVENGE,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
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
    // OpenEuler
    {
        .names = { "OpenEuler" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENEULER,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // OpenIndiana
    {
        .names = { "OpenIndiana" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENINDIANA,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_DEFAULT,
        },
    },
    // OpenMamba
    {
        .names = { "OpenMamba" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENMAMBA,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_GREEN,
        },
    },
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
    // OpenStage
    {
        .names = { "OpenStage" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSTAGE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
    },
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
    // openSuseMicroOS
    {
        .names = { "opensuse-microos" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_MICROOS,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
    },
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
    // OpenWrt
    {
        .names = { "openwrt" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPENWRT,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // OPNsense
    {
        .names = { "OPNsense" },
        .lines = FASTFETCH_DATATEXT_LOGO_OPNSENSE,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_256 "202",
        },
    },
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
    // Oreon
    {
        .names = { "Oreon" },
        .lines = FASTFETCH_DATATEXT_LOGO_OREON,
        .colors = {
            FF_COLOR_FG_DEFAULT,
            FF_COLOR_FG_DEFAULT,
        },
    },
    // OS/2 Warp
    {
        .names = { "OS2Warp" },
        .lines = FASTFETCH_DATATEXT_LOGO_OS2WARP,
        .colors = {
            FF_COLOR_FG_LIGHT_WHITE,
            FF_COLOR_FG_LIGHT_BLUE,
        },
    },
    // OS_Elbrus
    {
        .names = { "OS Elbrus" },
        .lines = FASTFETCH_DATATEXT_LOGO_OS_ELBRUS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // OSMC
    {
        .names = { "OSMC", "Open Source Media Center" },
        .lines = FASTFETCH_DATATEXT_LOGO_OSMC,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
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
    // LAST
    {},
};
