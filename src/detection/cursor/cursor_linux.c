#include "cursor.h"

#include "common/properties.h"
#include "common/parsing.h"
#include "common/settings.h"
#include "detection/gtk_qt/gtk_qt.h"
#include "detection/displayserver/displayserver.h"
#include "util/stringUtils.h"

#include <stdlib.h>

static bool detectCursorGTK(const FFinstance* instance, FFCursorResult* result)
{
    const FFGTKResult* gtk = ffDetectGTK4(instance);

    if(gtk->cursor.length == 0)
        gtk = ffDetectGTK3(instance);

    if(gtk->cursor.length == 0)
        gtk = ffDetectGTK2(instance);

    if(gtk->cursor.length == 0)
        return false;

    ffStrbufAppend(&result->theme, &gtk->cursor);
    ffStrbufAppend(&result->size, &gtk->cursorSize);
    return true;
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

    if(ffStrbufCompS(&wmde->wmPrettyName, FF_WM_PRETTY_WSLG) == 0)
        ffStrbufAppendS(&result->error, "WSLg uses native windows cursor");
    else if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, FF_WM_PROTOCOL_TTY) == 0)
        ffStrbufAppendS(&result->error, "Cursor isn't supported in TTY");
    else if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, FF_DE_PRETTY_PLASMA) == 0)
        detectCursorFromConfigFile(instance, "kcminputrc", "cursorTheme =", "Breeze", "cursorSize =", "24", result);
    else if(ffStrbufStartsWithIgnCaseS(&wmde->dePrettyName, FF_DE_PRETTY_LXQT))
        detectCursorFromConfigFile(instance, "lxqt/session.conf", "cursor_theme =", "Adwaita", "cursor_size =", "24", result);
    else if(
        !detectCursorGTK(instance, result) &&
        !detectCursorFromEnv(instance, result) &&
        !ffParsePropFileHome(instance, ".icons/default/index.theme", "Inherits =", &result->theme) &&
        !detectCursorFromXResources(instance, result) &&
        !ffParsePropFileData(instance, "icons/default/index.theme", "Inherits =", &result->theme)
    ) ffStrbufAppendS(&result->error, "Couldn't find cursor");
}
