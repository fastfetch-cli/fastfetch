#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo W[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_WII_LINUX
    // WiiLinux
    {
        .names = { "WiiLinuxNgx", "WiiLinux", "Wii-Linux", "Wii Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_WII_LINUX,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_WINDOWS_2025
    // Windows2025
    {
        .names = { "Windows Server 2025" },
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_2025,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_WINDOWS_11
    // Windows11
    {
        .names = { "Windows 11", "Windows Server 2022" },
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_11,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_WINDOWS_11_SMALL
    // Windows11Small
    {
        .names = { "Windows 11_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_11_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_WINDOWS_8
    // Windows8
    {
        .names = { "Windows 8", "Windows 8.1", "Windows 10", "Windows Server 2012", "Windows Server 2012 R2", "Windows Server 2016", "Windows Server 2019" },
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_8,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_WINDOWS
    // Windows
    {
        .names = { "Windows", "Windows 7", "Windows Server 2008", "Windows Server 2008 R2" },
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_WINDOWS_95
    // Windows95
    {
        .names = { "Windows 95", "Windows 9x" },
        .lines = FASTFETCH_DATATEXT_LOGO_WINDOWS_95,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_WOLFOS
    // WolfOS
    {
        .names = { "WolfOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_WOLFOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_GREEN,
        },
    },
    #endif
    // LAST
    {},
};
