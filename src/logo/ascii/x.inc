#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo X[] = {
    // XCP-ng
    {
        .names = { "XCP-ng", "xenenterprise" },
        .lines = FASTFETCH_DATATEXT_LOGO_XCP_NG,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_BLACK,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_YELLOW,
        } },
    // Xenia
    {
        .names = { "Xenia" },
        .lines = FASTFETCH_DATATEXT_LOGO_XENIA,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_LIGHT_BLACK,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Xenia_old
    {
        .names = { "Xenia_old" },
        .lines = FASTFETCH_DATATEXT_LOGO_XENIA_OLD,
        .type = FF_LOGO_LINE_TYPE_ALTER_BIT,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
        },
    },
    // XeroArch
    {
        .names = { "XeroArch" },
        .lines = FASTFETCH_DATATEXT_LOGO_XEROARCH,
        .colors = {
            FF_COLOR_FG_256 "50",
            FF_COLOR_FG_256 "14",
            FF_COLOR_FG_256 "50",
            FF_COLOR_FG_256 "93",
            FF_COLOR_FG_256 "16",
            FF_COLOR_FG_256 "15",
        },
    },
    // Xferience
    {
        .names = { "Xferience" },
        .lines = FASTFETCH_DATATEXT_LOGO_XFERIENCE,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    // Xubuntu
    {
        .names = { "Xubuntu" },
        .lines = FASTFETCH_DATATEXT_LOGO_XUBUNTU,
        .colors = {
            FF_COLOR_FG_256 "25",
            FF_COLOR_FG_DEFAULT,
        },
    },
    // Xray_OS
    {
        .names = { "Xray_OS" },
        .lines = FASTFETCH_DATATEXT_LOGO_XRAY_OS,
        .colors = {
            FF_COLOR_FG_256 "15",
            FF_COLOR_FG_256 "14",
            FF_COLOR_FG_256 "16",
        },
    },
    // Xinux
    {
        .names = { "Xinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_XINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
        },
    },
    // XJ380
    {
        .names = { "XJ380" },
        .lines = FASTFETCH_DATATEXT_LOGO_XJ380,
        .colors = {
            FF_COLOR_FG_RGB "0;162;232",
            FF_COLOR_FG_RGB "255;242;0",
        },
    },
    // Ximper
    {
        .names = { "Ximper" },
        .lines = FASTFETCH_DATATEXT_LOGO_XIMPER,
        .colors = {
            FF_COLOR_FG_256 "21",
            FF_COLOR_FG_256 "57",
            FF_COLOR_FG_256 "92",
            FF_COLOR_FG_256 "128",
        },
    },
    // LAST
    {},
};
