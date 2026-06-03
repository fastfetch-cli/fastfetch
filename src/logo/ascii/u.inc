#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo U[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_UBLINUX
    // UBLinux
    {
        .names = { "UBLinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBLINUX,
        .colors = {
            FF_COLOR_FG_256 "38",
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_256 "38",
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBLINUX_SMALL
    // UBLinuxSmall
    {
        .names = { "UBLinux_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_UBLINUX_SMALL,
        .colors = {
            FF_COLOR_FG_256 "38",
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_256 "38",
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBUNTU
    // Ubuntu
    {
        .names = { "ubuntu", "ubuntu-linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_RED,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBUNTU_SMALL
    // UbuntuSmall
    {
        .names = { "ubuntu_small", "ubuntu-linux_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_RED,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBUNTU_OLD
    // UbuntuOld
    {
        .names = { "ubuntu_old", "ubuntu-linux_old" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_OLD,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBUNTU_OLD2
    // UbuntuOld2
    {
        .names = { "ubuntu_old2" },
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_OLD2,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBUNTU_OLD2_SMALL
    // UbuntuOld2Small
    {
        .names = { "ubuntu_old2_small", "ubuntu_old2_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_OLD2_SMALL,
        .colors = {
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBUNTU_BUDGIE
    // UbuntuBudgie
    {
        .names = { "ubuntu budgie", "ubuntu-budgie" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_BUDGIE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBUNTU_CINNAMON
    // UbuntuCinnamon
    {
        .names = { "ubuntu cinnamon", "ubuntu-cinnamon" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_CINNAMON,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBUNTU_GNOME
    // UbuntuGNOME
    {
        .names = { "ubuntu gnome", "ubuntu-gnome" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_GNOME,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBUNTU_KYLIN
    // UbuntuKylin
    {
        .names = { "ubuntu kylin", "ubuntu-kylin" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_KYLIN,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBUNTU_MATE
    // UbuntuMate
    {
        .names = { "ubuntu mate", "ubuntu-mate" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_MATE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBUNTU_STUDIO
    // UbuntuStudio
    {
        .names = { "ubuntu studio", "ubuntu-studio" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_STUDIO,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBUNTU_SWAY
    // UbuntuSway
    {
        .names = { "ubuntu sway", "ubuntu-sway" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_SWAY,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBUNTU_TOUCH
    // UbuntuTouch
    {
        .names = { "ubuntu touch", "ubuntu-touch" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_TOUCH,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UBUNTU_UNITY
    // UbuntuUnity
    {
        .names = { "ubuntu unity", "ubuntu-unity" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_UNITY,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ULTRAMARINE
    // Ultramarine
    {
        .names = { "Ultramarine", "Ultramarine Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_ULTRAMARINE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_ULTRAMARINE_SMALL
    // Ultramarine Small
    {
        .names = { "Ultramarine_small" },
        .lines = FASTFETCH_DATATEXT_LOGO_ULTRAMARINE_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UNIFI
    // Unifi
    {
        .names = { "Unifi" },
        .lines = FASTFETCH_DATATEXT_LOGO_UNIFI,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UNIVALENT
    // Univalent
    {
        .names = { "Univalent" },
        .lines = FASTFETCH_DATATEXT_LOGO_UNIVALENT,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UNIVENTION
    // Univention
    {
        .names = { "Univention" },
        .lines = FASTFETCH_DATATEXT_LOGO_UNIVENTION,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UOS
    // UOS
    {
        .names = { "UOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_UOS,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_URUKOS
    // UrukOS
    {
        .names = { "UrukOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_URUKOS,
        .colors = {
            FF_COLOR_FG_LIGHT_BLUE,
            FF_COLOR_FG_LIGHT_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_BLUE,
            FF_COLOR_FG_BLUE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UWUNTU
    // Uwuntu
    {
        .names = { "uwuntu" },
        .lines = FASTFETCH_DATATEXT_LOGO_UWUNTU,
        .colors = {
            FF_COLOR_FG_256 "225",
            FF_COLOR_FG_256 "206",
            FF_COLOR_FG_256 "52",
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_UZBEK
    // Uzbek
    {
        .names = { "Uzbek" },
        .lines = FASTFETCH_DATATEXT_LOGO_UZBEK,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
    },
    #endif
    // LAST
    {},
};
