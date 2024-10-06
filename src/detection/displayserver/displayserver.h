#pragma once

#include "fastfetch.h"

#define FF_DE_PRETTY_PLASMA "KDE Plasma"
#define FF_DE_PRETTY_GNOME "GNOME"
#define FF_DE_PRETTY_GNOME_CLASSIC "GNOME Classic"
#define FF_DE_PRETTY_XFCE4 "Xfce4"
#define FF_DE_PRETTY_CINNAMON "Cinnamon"
#define FF_DE_PRETTY_MATE "Mate"
#define FF_DE_PRETTY_LXDE "LXDE"
#define FF_DE_PRETTY_LXQT "LXQt"
#define FF_DE_PRETTY_BUDGIE "Budgie"
#define FF_DE_PRETTY_CDE "CDE"
#define FF_DE_PRETTY_UNITY "Unity"
#define FF_DE_PRETTY_UKUI "UKUI"

#define FF_WM_PRETTY_KWIN "KWin"
#define FF_WM_PRETTY_MUTTER "Mutter"
#define FF_WM_PRETTY_MUFFIN "Muffin"
#define FF_WM_PRETTY_MARCO "Marco"
#define FF_WM_PRETTY_XFWM4 "Xfwm4"
#define FF_WM_PRETTY_OPENBOX "Openbox"
#define FF_WM_PRETTY_I3 "i3"
#define FF_WM_PRETTY_HYPRLAND "Hyprland"
#define FF_WM_PRETTY_WAYFIRE "Wayfire"
#define FF_WM_PRETTY_SWAY "Sway"
#define FF_WM_PRETTY_BSPWM "bspwm"
#define FF_WM_PRETTY_DWM "dwm"
#define FF_WM_PRETTY_WESTON "Weston"
#define FF_WM_PRETTY_XMONAD "XMonad"
#define FF_WM_PRETTY_WSLG "WSLg"
#define FF_WM_PRETTY_TINYWM "TinyWM"
#define FF_WM_PRETTY_QTILE "Qtile"
#define FF_WM_PRETTY_HERBSTLUFTWM "herbstluftwm"
#define FF_WM_PRETTY_ICEWM "IceWM"
#define FF_WM_PRETTY_SPECTRWM "spectrwm"
#define FF_WM_PRETTY_DTWM "dtwm"


#define FF_WM_PROTOCOL_TTY "TTY"
#define FF_WM_PROTOCOL_X11 "X11"
#define FF_WM_PROTOCOL_WAYLAND "Wayland"

typedef enum FFDisplayType {
    FF_DISPLAY_TYPE_UNKNOWN,
    FF_DISPLAY_TYPE_BUILTIN,
    FF_DISPLAY_TYPE_EXTERNAL,
} FFDisplayType;

typedef enum FFDisplayHdrStatus
{
    FF_DISPLAY_HDR_STATUS_UNKNOWN,
    FF_DISPLAY_HDR_STATUS_UNSUPPORTED,
    FF_DISPLAY_HDR_STATUS_SUPPORTED,
    FF_DISPLAY_HDR_STATUS_ENABLED,
} FFDisplayHdrStatus;

typedef struct FFDisplayResult
{
    uint32_t width;
    uint32_t height;
    double refreshRate;
    uint32_t scaledWidth;
    uint32_t scaledHeight;
    FFstrbuf name;
    FFDisplayType type;
    uint32_t rotation;
    uint64_t id; // platform dependent
    uint32_t physicalWidth;
    uint32_t physicalHeight;
    bool primary;
    uint8_t bitDepth;
    FFDisplayHdrStatus hdrStatus;
    uint16_t manufactureYear;
    uint16_t manufactureWeek;
    uint32_t serial;
} FFDisplayResult;

typedef struct FFDisplayServerResult
{
    FFstrbuf wmProcessName;
    FFstrbuf wmPrettyName;
    FFstrbuf wmProtocolName;
    FFstrbuf deProcessName;
    FFstrbuf dePrettyName;
    FFlist displays; //List of FFDisplayResult
} FFDisplayServerResult;

const FFDisplayServerResult* ffConnectDisplayServer();

FFDisplayResult* ffdsAppendDisplay(
    FFDisplayServerResult* result,
    uint32_t width,
    uint32_t height,
    double refreshRate,
    uint32_t scaledWidth,
    uint32_t scaledHeight,
    uint32_t rotation,
    FFstrbuf* name,
    FFDisplayType type,
    bool primary,
    uint64_t id,
    uint32_t physicalWidth,
    uint32_t physicalHeight);
