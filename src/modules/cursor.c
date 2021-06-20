#include "fastfetch.h"

#define FF_CURSOR_MODULE_NAME "Cursor"
#define FF_CURSOR_NUM_FORMAT_ARGS 2

static void printCursor(FFinstance* instance, const FFstrbuf* cursorTheme, const FFstrbuf* cursorSize)
{
    ffPrintLogoAndKey(instance, FF_CURSOR_MODULE_NAME, 0, &instance->config.cursorKey);
    ffStrbufWriteTo(cursorTheme, stdout);

    if(cursorSize->length > 0)
    {
        fputs(" (", stdout);
        ffStrbufWriteTo(cursorSize, stdout);
        fputs("pt)", stdout);
    }

    putchar('\n');
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

    printCursor(instance, &gtk->cursor, &gtk->cursorSize);
}

static void printCursorPlasma(FFinstance* instance)
{
    FFstrbuf cursorTheme;
    ffStrbufInit(&cursorTheme);

    FFstrbuf cursorSize;
    ffStrbufInit(&cursorSize);

    if(ffParsePropFileConfigValues(instance, "kcminputrc", 2, (FFpropquery[]) {
        {"cursorTheme =", &cursorTheme},
        {"cursorSize =", &cursorSize}
    })) {
        if(cursorTheme.length == 0)
            ffStrbufAppendS(&cursorTheme, "Breeze");

        if(cursorSize.length == 0)
            ffStrbufAppendS(&cursorSize, "24");
    }

    if(cursorTheme.length == 0)
    {
        ffPrintError(instance, FF_CURSOR_MODULE_NAME, 0, &instance->config.cursorKey, &instance->config.cursorFormat, FF_CURSOR_NUM_FORMAT_ARGS, "Couldn't find plasma cursor in kcminputrc");
        return;
    }

    printCursor(instance, &cursorTheme, &cursorSize);
    ffStrbufDestroy(&cursorTheme);
    ffStrbufDestroy(&cursorSize);
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

void ffPrintCursor(FFinstance* instance)
{
    const FFWMDEResult* wmde = ffDetectWMDE(instance);

    if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, "KDE Plasma") == 0)
        printCursorPlasma(instance);
    else if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, "XFCE") == 0)
        printCursorXFCE(instance);
    else
        printCursorGTK(instance);
}
