#include "fastfetch.h"

#define FF_CURSOR_MODULE_NAME "Cursor"
#define FF_CURSOR_NUM_FORMAT_ARGS 2

static void printCursor(FFinstance* instance, FFstrbuf* cursorTheme, const FFstrbuf* cursorSize)
{
    ffStrbufRemoveIgnCaseEndS(cursorTheme, "cursors");
    ffStrbufRemoveIgnCaseEndS(cursorTheme, "cursor");
    ffStrbufTrimRight(cursorTheme, '_');
    ffStrbufTrimRight(cursorTheme, '-');
    if(cursorTheme->length == 0)
        ffStrbufAppendS(cursorTheme, "default");

    if(instance->config.cursorFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_CURSOR_MODULE_NAME, 0, &instance->config.cursorKey);
        ffStrbufWriteTo(cursorTheme, stdout);

        if(cursorSize != NULL && cursorSize->length > 0)
        {
            fputs(" (", stdout);
            ffStrbufWriteTo(cursorSize, stdout);
            fputs("px)", stdout);
        }

        putchar('\n');
    }
    else
    {
        ffPrintFormatString(instance, FF_CURSOR_MODULE_NAME, 0, &instance->config.cursorKey, &instance->config.cursorFormat, NULL, FF_CURSOR_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, cursorTheme},
            {FF_FORMAT_ARG_TYPE_STRBUF, cursorSize}
        });
    }
}

static void printCursorGTK(FFinstance* instance)
{
    const FFGTKResult* gtk = ffDetectGTK4(instance);

    if(gtk->cursor.length == 0)
        gtk = ffDetectGTK3(instance);

    if(gtk->cursor.length == 0)
        gtk = ffDetectGTK2(instance);

    if(gtk->cursor.length == 0)
    {
        ffPrintError(instance, FF_CURSOR_MODULE_NAME, 0, &instance->config.cursorKey, &instance->config.cursorFormat, FF_CURSOR_NUM_FORMAT_ARGS, "Couldn't detect GTK Cursor");
        return;
    }

    //gtk->cursor is const, so we don't want to modify it
    FFstrbuf theme;
    ffStrbufInitCopy(&theme, &gtk->cursor);

    printCursor(instance, &theme, &gtk->cursorSize);

    ffStrbufDestroy(&theme);
}

static void printCursorXFCE(FFinstance* instance)
{
    FFstrbuf cursorTheme;
    ffStrbufInit(&cursorTheme);

    ffStrbufAppendS(&cursorTheme, ffSettingsGetXFConf(instance, "xsettings", "/Gtk/CursorThemeName", FF_VARIANT_TYPE_STRING).strValue);

    if(cursorTheme.length == 0)
    {
        ffPrintError(instance, FF_CURSOR_MODULE_NAME, 0, &instance->config.cursorKey, &instance->config.cursorFormat, FF_CURSOR_NUM_FORMAT_ARGS, "Couldn't find xfce cursor in xfconf (xsettings::/Gtk/CursorThemeName)");
        return;
    }

    FFstrbuf cursorSize;
    ffStrbufInit(&cursorSize);
    int cursorSizeVal = ffSettingsGetXFConf(instance, "xsettings", "/Gtk/CursorThemeSize", FF_VARIANT_TYPE_INT).intValue;
    if(cursorSizeVal > 0)
        ffStrbufAppendF(&cursorSize, "%i", cursorSizeVal);

    printCursor(instance, &cursorTheme, &cursorSize);
    ffStrbufDestroy(&cursorTheme);
    ffStrbufDestroy(&cursorSize);
}

static void printCursorFromConfigFile(FFinstance* instance, const char* relativeFilePath, const char* themeStart, const char* themeDefault, const char* sizeStart, const char* sizeDefault)
{
    FFstrbuf cursorTheme;
    ffStrbufInit(&cursorTheme);

    FFstrbuf cursorSize;
    ffStrbufInit(&cursorSize);

    if(ffParsePropFileConfigValues(instance, relativeFilePath, 2, (FFpropquery[]) {
        {themeStart, &cursorTheme},
        {sizeStart, &cursorSize}
    })) {

        if(cursorTheme.length == 0)
            ffStrbufAppendS(&cursorTheme, themeDefault);

        if(cursorSize.length == 0)
            ffStrbufAppendS(&cursorSize, sizeDefault);
    }

    if(cursorTheme.length == 0)
        ffPrintError(instance, FF_CURSOR_MODULE_NAME, 0, &instance->config.cursorKey, &instance->config.cursorFormat, FF_CURSOR_NUM_FORMAT_ARGS, "Couldn't find cursor in %s", relativeFilePath);
    else
        printCursor(instance, &cursorTheme, &cursorSize);

    ffStrbufDestroy(&cursorTheme);
    ffStrbufDestroy(&cursorSize);
}

static bool printCursorFromXResources(FFinstance* instance)
{
    FFstrbuf theme;
    ffStrbufInit(&theme);

    FFstrbuf size;
    ffStrbufInit(&size);

    ffParsePropFileHomeValues(instance, ".Xresources", 2, (FFpropquery[]) {
        {"Xcursor.theme :", &theme},
        {"Xcursor.size :", &size}
    });

    if(theme.length == 0)
    {
        ffStrbufDestroy(&size);
        ffStrbufDestroy(&theme);
        return false;
    }

    printCursor(instance, &theme, &size);
    ffStrbufDestroy(&size);
    ffStrbufDestroy(&theme);
    return true;
}

static bool printCursorFromXDG(FFinstance* instance, bool user)
{
    FFstrbuf theme;
    ffStrbufInit(&theme);

    if(user)
        ffParsePropFileHome(instance, ".icons/default/index.theme", "Inherits =", &theme);
    else
        ffParsePropFile("/usr/share/icons/default/index.theme", "Inherits =", &theme);

    if(theme.length == 0)
    {
        ffStrbufDestroy(&theme);
        return false;
    }

    printCursor(instance, &theme, NULL);
    ffStrbufDestroy(&theme);
    return true;
}

static bool printCursorFromEnv(FFinstance* instance)
{
    const char* xcursor_theme = getenv("XCURSOR_THEME");

    if(!ffStrSet(xcursor_theme))
        return false;

    FFstrbuf theme;
    ffStrbufInit(&theme);
    ffStrbufAppendS(&theme, xcursor_theme);

    FFstrbuf size;
    ffStrbufInit(&size);
    ffStrbufAppendS(&size, getenv("XCURSOR_SIZE"));

    printCursor(instance, &theme, &size);

    ffStrbufDestroy(&size);
    ffStrbufDestroy(&theme);
    return true;
}

void ffPrintCursor(FFinstance* instance)
{
    #ifdef __ANDROID__
        ffPrintError(instance, FF_CURSOR_MODULE_NAME, 0, &instance->config.cursorKey, &instance->config.cursorFormat, FF_CURSOR_NUM_FORMAT_ARGS, "Cursor detection is not supported on Android");
        return;
    #endif

    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, "TTY") == 0)
    {
        ffPrintError(instance, FF_CURSOR_MODULE_NAME, 0, &instance->config.cursorKey, &instance->config.cursorFormat, FF_CURSOR_NUM_FORMAT_ARGS, "Cursor isn't supported in TTY");
        return;
    }

    if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, "KDE Plasma") == 0)
    {
        printCursorFromConfigFile(instance, "kcminputrc", "cursorTheme =", "Breeze", "cursorSize =", "24");
        return;
    }

    if(ffStrbufStartsWithIgnCaseS(&wmde->dePrettyName, "XFCE"))
    {
        printCursorXFCE(instance);
        return;
    }

    if(ffStrbufStartsWithIgnCaseS(&wmde->dePrettyName, "LXQt"))
    {
        printCursorFromConfigFile(instance, "lxqt/session.conf", "cursor_theme =", "Adwaita", "cursor_size =", "24");
        return;
    }

    if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, "Gnome") == 0 || ffStrbufIgnCaseCompS(&wmde->dePrettyName, "Cinnamon") == 0 || ffStrbufIgnCaseCompS(&wmde->dePrettyName, "Mate") == 0)
    {
        printCursorGTK(instance);
        return;
    }

    if(
        printCursorFromEnv(instance) ||
        printCursorFromXDG(instance, true) ||
        printCursorFromXResources(instance) ||
        printCursorFromXDG(instance, false)
    ) return;

    printCursorGTK(instance);
}
