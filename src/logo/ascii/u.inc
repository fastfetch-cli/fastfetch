#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo U[] = {
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
    // Ubuntu
    {
        .names = { "ubuntu", "ubuntu-linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_RED,
        },
    },
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
    // UbuntuCinnamon
    {
        .names = { "ubuntu cinnamon", "ubuntu-cinnamon" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_CINNAMON,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
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
    // UbuntuStudio
    {
        .names = { "ubuntu studio", "ubuntu-studio" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_STUDIO,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // UbuntuSway
    {
        .names = { "ubuntu sway", "ubuntu-sway" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_SWAY,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // UbuntuTouch
    {
        .names = { "ubuntu touch", "ubuntu-touch" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_TOUCH,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    // UbuntuUnity
    {
        .names = { "ubuntu unity", "ubuntu-unity" },
        .lines = FASTFETCH_DATATEXT_LOGO_UBUNTU_UNITY,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
    },
    // Ultramarine
    {
        .names = { "Ultramarine", "Ultramarine Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_ULTRAMARINE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Ultramarine Small
    {
        .names = { "Ultramarine_small" },
        .lines = FASTFETCH_DATATEXT_LOGO_ULTRAMARINE_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Unifi
    {
        .names = { "Unifi" },
        .lines = FASTFETCH_DATATEXT_LOGO_UNIFI,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Univalent
    {
        .names = { "Univalent" },
        .lines = FASTFETCH_DATATEXT_LOGO_UNIVALENT,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    // Univention
    {
        .names = { "Univention" },
        .lines = FASTFETCH_DATATEXT_LOGO_UNIVENTION,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
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
    // Uzbek
    {
        .names = { "Uzbek" },
        .lines = FASTFETCH_DATATEXT_LOGO_UZBEK,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
    },
    // LAST
    {},
};
