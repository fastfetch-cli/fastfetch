#include "cursor.h"

#include "common/properties.h"
#include "common/parsing.h"
#include "common/settings.h"
#include "detection/gtk.h"
#include "detection/displayserver/displayserver.h"

#include <stdlib.h>

static void detectCursorGTK(const FFinstance* instance, FFCursorResult* result)
{
    const FFGTKResult* gtk = ffDetectGTK4(instance);

    if(gtk->cursor.length == 0)
        gtk = ffDetectGTK3(instance);

    if(gtk->cursor.length == 0)
        gtk = ffDetectGTK2(instance);

    if(gtk->cursor.length == 0)
    {
        ffStrbufAppendS(&result->error, "Couldn't detect GTK Cursor");
        return;
    }

    ffStrbufAppend(&result->theme, &gtk->cursor);
    ffStrbufAppend(&result->size, &gtk->cursorSize);
}

static void detectCursorXFCE(const FFinstance* instance, FFCursorResult* result)
{
    ffStrbufAppendS(&result->theme, ffSettingsGetXFConf(instance, "xsettings", "/Gtk/CursorThemeName", FF_VARIANT_TYPE_STRING).strValue);

    if(result->theme.length == 0)
        ffStrbufAppendS(&result->error, "Couldn't find xfce cursor in xfconf (xsettings::/Gtk/CursorThemeName)");

    int cursorSizeVal = ffSettingsGetXFConf(instance, "xsettings", "/Gtk/CursorThemeSize", FF_VARIANT_TYPE_INT).intValue;
    if(cursorSizeVal > 0)
        ffStrbufAppendF(&result->size, "%i", cursorSizeVal);
}

static void detectCursorFromConfigFile(const FFinstance* instance, const char* relativeFilePath, const char* themeStart, const char* themeDefault, const char* sizeStart, const char* sizeDefault, FFCursorResult* result)
{
    if(ffParsePropFileConfigValues(instance, relativeFilePath, 2, (FFpropquery[]) {
        {themeStart, &result->theme},
        {sizeStart, &result->size}
    })) {

        if(result->theme.length == 0)
            ffStrbufAppendS(&result->theme, themeDefault);

        if(result->size.length == 0)
            ffStrbufAppendS(&result->size, sizeDefault);
    }

    if(result->theme.length == 0)
        ffStrbufAppendF(&result->error, "Couldn't find cursor in %s", relativeFilePath);
}

static bool detectCursorFromXResources(const FFinstance* instance, FFCursorResult* result)
{
    ffParsePropFileHomeValues(instance, ".Xresources", 2, (FFpropquery[]) {
        {"Xcursor.theme :", &result->theme},
        {"Xcursor.size :", &result->size}
    });

    return result->theme.length > 0;
}

static bool detectCursorFromXDG(const FFinstance* instance, bool user, FFCursorResult* result)
{
    if(user)
        ffParsePropFileHome(instance, ".icons/default/index.theme", "Inherits =", &result->theme);
    else
        ffParsePropFileData(instance, "icons/default/index.theme", "Inherits =", &result->theme);

    return result->theme.length > 0;
}

static bool detectCursorFromEnv(const FFinstance* instance, FFCursorResult* result)
{
    FF_UNUSED(instance);
    const char* xcursor_theme = getenv("XCURSOR_THEME");

    if(!ffStrSet(xcursor_theme))
        return false;

    ffStrbufAppendS(&result->theme, xcursor_theme);
    ffStrbufAppendS(&result->size, getenv("XCURSOR_SIZE"));

    return true;
}

void ffDetectCursor(const FFinstance* instance, FFCursorResult* result)
{
    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufCompS(&wmde->wmPrettyName, "WSLg") == 0)
    {
        ffStrbufAppendS(&result->error, "WSLg uses native windows cursor");
        return;
    }

    if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, "TTY") == 0)
    {
        ffStrbufAppendS(&result->error, "Cursor isn't supported in TTY");
        return;
    }

    if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, "KDE Plasma") == 0)
        return detectCursorFromConfigFile(instance, "kcminputrc", "cursorTheme =", "Breeze", "cursorSize =", "24", result);

    if(ffStrbufStartsWithIgnCaseS(&wmde->dePrettyName, "XFCE"))
        return detectCursorXFCE(instance, result);

    if(ffStrbufStartsWithIgnCaseS(&wmde->dePrettyName, "LXQt"))
        return detectCursorFromConfigFile(instance, "lxqt/session.conf", "cursor_theme =", "Adwaita", "cursor_size =", "24", result);

    if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, "Gnome") == 0 || ffStrbufIgnCaseCompS(&wmde->dePrettyName, "Cinnamon") == 0 || ffStrbufIgnCaseCompS(&wmde->dePrettyName, "Mate") == 0)
        return detectCursorGTK(instance, result);

    if(
        detectCursorFromEnv(instance, result) ||
        detectCursorFromXDG(instance, true, result) ||
        detectCursorFromXResources(instance, result) ||
        detectCursorFromXDG(instance, false, result)
    ) return;

    detectCursorGTK(instance, result);
}
