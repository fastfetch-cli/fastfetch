#pragma once

#ifndef FF_INCLUDED_detection_displayserver
#define FF_INCLUDED_detection_displayserver

#include "fastfetch.h"

#define FF_DE_PRETTY_PLASMA "KDE Plasma"
#define FF_DE_PRETTY_GNOME "Gnome"
#define FF_DE_PRETTY_GNOME_CLASSIC "Gnome Classic"
#define FF_DE_PRETTY_XFCE4 "Xfce4"
#define FF_DE_PRETTY_CINNAMON "Cinnamon"
#define FF_DE_PRETTY_MATE "Mate"
#define FF_DE_PRETTY_LXDE "LXDE"
#define FF_DE_PRETTY_LXQT "LXQT"
#define FF_DE_PRETTY_BUDGIE "Budgie"
#define FF_DE_PRETTY_UNITY "Unity"

#define FF_WM_PRETTY_KWIN "KWin"
#define FF_WM_PRETTY_MUTTER "Mutter"
#define FF_WM_PRETTY_MUFFIN "Muffin"
#define FF_WM_PRETTY_MARCO "Marco"
#define FF_WM_PRETTY_XFWM4 "Xfwm4"
#define FF_WM_PRETTY_OPENBOX "Openbox"
#define FF_WM_PRETTY_I3 "i3"
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


#define FF_WM_PROTOCOL_TTY "TTY"
#define FF_WM_PROTOCOL_X11 "X11"
#define FF_WM_PROTOCOL_WAYLAND "Wayland"

typedef enum FFDisplayType {
    FF_DISPLAY_TYPE_UNKNOWN,
    FF_DISPLAY_TYPE_BUILTIN,
    FF_DISPLAY_TYPE_EXTERNAL,
} FFDisplayType;

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
    bool primary;
} FFDisplayResult;

typedef struct FFDisplayServerResult
{
    FFstrbuf wmProcessName;
    FFstrbuf wmPrettyName;
    FFstrbuf wmProtocolName;
    FFstrbuf deProcessName;
    FFstrbuf dePrettyName;
    FFstrbuf deVersion;
    FFlist displays; //List of FFDisplayResult
} FFDisplayServerResult;

const FFDisplayServerResult* ffConnectDisplayServer();

bool ffdsAppendDisplay(
    FFDisplayServerResult* result,
    uint32_t width,
    uint32_t height,
    double refreshRate,
    uint32_t scaledWidth,
    uint32_t scaledHeight,
    uint32_t rotation,
    FFstrbuf* name,
    FFDisplayType type,
    bool primary);

#endif
