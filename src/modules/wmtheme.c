#include "fastfetch.h"
#include "common/properties.h"
#include "common/printing.h"
#include "common/parsing.h"
#include "common/settings.h"
#include "detection/gtk.h"
#include "detection/displayserver/displayserver.h"

#include <stdlib.h>
#include <string.h>

#define FF_WMTHEME_MODULE_NAME "WM Theme"
#define FF_WMTHEME_NUM_FORMAT_ARGS 1

static void printWMTheme(FFinstance* instance, const char* theme)
{
    if(instance->config.wmTheme.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme.key);
        puts(theme);
    }
    else
    {
        ffPrintFormat(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, FF_WMTHEME_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, theme}
        });
    }
}

static void printWMThemeFromConfigFile(FFinstance* instance, const char* configFile, const char* themeRegex, const char* defaultValue)
{
    FFstrbuf theme;
    ffStrbufInit(&theme);

    if(!ffParsePropFileConfig(instance, configFile, themeRegex, &theme))
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "Config file %s doesn't exist", configFile);
        ffStrbufDestroy(&theme);
        return;
    }

    if(theme.length == 0)
    {
        ffStrbufDestroy(&theme);

        if(defaultValue == NULL)
        {
            ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "Couldn't find WM theme in %s", configFile);
            return;
        }

        printWMTheme(instance, defaultValue);
        return;
    }

    // Remove Plasma-generated prefixes
    uint32_t idx = 0;

    idx = ffStrbufFirstIndexS(&theme, "qml_");
    if(idx != theme.length)
        ffStrbufSubstrAfter(&theme, idx + 3);

    idx = ffStrbufFirstIndexS(&theme, "svg__");
    if(idx != theme.length)
        ffStrbufSubstrAfter(&theme, idx + 4);

    printWMTheme(instance, theme.chars);
    ffStrbufDestroy(&theme);
}

static void printWMThemeFromSettings(FFinstance* instance, const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey)
{
    const char* theme = ffSettingsGet(instance, dconfKey, gsettingsSchemaName, gsettingsPath, gsettingsKey, FF_VARIANT_TYPE_STRING).strValue;

    if(!ffStrSet(theme))
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "Couldn't find WM theme in DConf or GSettings");
        return;
    }

    printWMTheme(instance, theme);
}

static void printGTKThemeAsWMTheme(FFinstance* instance)
{
    const FFGTKResult* gtk = ffDetectGTK4(instance);

    if(gtk->theme.length > 0)
    {
        printWMTheme(instance, gtk->theme.chars);
        return;
    }

    gtk = ffDetectGTK3(instance);

    if(gtk->theme.length > 0)
    {
        printWMTheme(instance, gtk->theme.chars);
        return;
    }

    gtk = ffDetectGTK2(instance);

    if(gtk->theme.length > 0)
    {
        printWMTheme(instance, gtk->theme.chars);
        return;
    }

    ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "Couldn't detect GTK4/3/2 theme");
}

static void printMutter(FFinstance* instance)
{
    const char* theme = ffSettingsGet(instance, "/org/gnome/shell/extensions/user-theme/name", "org.gnome.shell.extensions.user-theme", NULL, "name", FF_VARIANT_TYPE_STRING).strValue;
    if(ffStrSet(theme))
    {
        printWMTheme(instance, theme);
        return;
    }

    printGTKThemeAsWMTheme(instance);
}

static void printMuffin(FFinstance* instance)
{
    const char* name = ffSettingsGet(instance, "/org/cinnamon/theme/name", "org.cinnamon.theme", NULL, "name", FF_VARIANT_TYPE_STRING).strValue;
    const char* theme = ffSettingsGet(instance, "/org/cinnamon/desktop/wm/preferences/theme", "org.cinnamon.desktop.wm.preferences", NULL, "theme", FF_VARIANT_TYPE_STRING).strValue;

    if(name == NULL && theme == NULL)
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "Couldn't find muffin theme in GSettings / DConf");
        return;
    }

    if(name == NULL)
        printWMTheme(instance, theme);
    else if(theme == NULL)
        printWMTheme(instance, name);
    else
    {
        FFstrbuf buffer;
        ffStrbufInit(&buffer);
        ffStrbufAppendS(&buffer, name);
        ffStrbufAppendS(&buffer, " (");
        ffStrbufAppendS(&buffer, theme);
        ffStrbufAppendC(&buffer, ')');

        printWMTheme(instance, buffer.chars);

        ffStrbufDestroy(&buffer);
    }
}

static void printXFWM4(FFinstance* instance)
{
    const char* theme = ffSettingsGetXFConf(instance, "xfwm4", "/general/theme", FF_VARIANT_TYPE_STRING).strValue;

    if(theme == NULL)
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "Couldn't find xfwm4::/general/theme in XFConf");
        return;
    }

    printWMTheme(instance, theme);
}

static void printOpenbox(FFinstance* instance, const FFstrbuf* dePrettyName)
{
    FFstrbuf absolutePath;
    ffStrbufInitA(&absolutePath, 64);
    ffStrbufAppendS(&absolutePath, instance->state.passwd->pw_dir);
    ffStrbufAppendC(&absolutePath, '/');

    if(ffStrbufIgnCaseCompS(dePrettyName, "LXQT") == 0)
        ffStrbufAppendS(&absolutePath, ".config/openbox/lxqt-rc.xml");
    else if(ffStrbufIgnCaseCompS(dePrettyName, "LXDE") == 0)
        ffStrbufAppendS(&absolutePath, ".config/openbox/lxde-rc.xml");
    else
        ffStrbufAppendS(&absolutePath, ".config/openbox/rc.xml");

    char* line = NULL;
    size_t len = 0;

    FILE* file = fopen(absolutePath.chars, "r");
    if(file == NULL)
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "Couldn't open \"%s\"", absolutePath.chars);
        ffStrbufDestroy(&absolutePath);

        return;
    }

    FFstrbuf theme;
    ffStrbufInitA(&theme, 256);

    while(getline(&line, &len, file) != -1)
    {
        if(strstr(line, "<theme>") != 0)
            break;
    }

    while(getline(&line, &len, file) != -1)
    {
        if(strstr(line, "<name>") != 0)
        {
            ffStrbufAppendS(&theme, line);
            ffStrbufRemoveStrings(&theme, 2, "<name>", "</name>");
            ffStrbufTrimRight(&theme, '\n');
            ffStrbufTrim(&theme, ' ');
            break;
        }

        if(strstr(line, "</theme>") != 0) // sanity check
            break;
    }

    if(line != NULL)
        free(line);

    fclose(file);

    if(theme.length == 0)
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "Couldn't find theme name in \"%s\"", absolutePath.chars);
    else
        printWMTheme(instance, theme.chars);

    ffStrbufDestroy(&theme);
    ffStrbufDestroy(&absolutePath);
}

#ifdef FF_HAVE_LIBPLIST
#include "common/library.h"
#include "common/io.h"
#include <plist/plist.h>

static const char* quartzCompositorParsePlist(FFinstance* instance, const FFstrbuf* content, FFstrbuf* theme) {
    FF_LIBRARY_LOAD(libplist, &instance->config.libplist, "dlopen libplist failed", "libplist-2.0"FF_LIBRARY_EXTENSION, 2);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_is_binary);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_from_bin);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_from_xml);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_dict_get_item);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_get_string_ptr);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_get_uint_val);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_free);

    plist_t root_node = NULL;
    if (ffplist_is_binary(content->chars, content->length))
        ffplist_from_bin(content->chars, content->length, &root_node);
    else
        ffplist_from_xml(content->chars, content->length, &root_node);

    if(root_node == NULL)
        return "parsing Quartz Compositor preference file failed";

    plist_t pWmThemeColor = ffplist_dict_get_item(root_node, "AppleAccentColor");
    uint64_t wmThemeColor = 100;
    if (pWmThemeColor != NULL)
        ffplist_get_uint_val(pWmThemeColor, &wmThemeColor);
    switch (wmThemeColor)
    {
        case (uint64_t)-1: ffStrbufAppendS(theme, " (Graphite)");
        case 0: ffStrbufAppendS(theme, "Red"); break;
        case 1: ffStrbufAppendS(theme, "Orange"); break;
        case 2: ffStrbufAppendS(theme, "Yellow"); break;
        case 3: ffStrbufAppendS(theme, "Green"); break;
        case 5: ffStrbufAppendS(theme, "Purple"); break;
        case 6: ffStrbufAppendS(theme, "Pink"); break;
        default: ffStrbufAppendS(theme, "Blue"); break;
    }

    plist_t pWmTheme = ffplist_dict_get_item(root_node, "AppleInterfaceStyle");
    ffStrbufAppendF(theme, " (%s)", pWmTheme != NULL ? ffplist_get_string_ptr(pWmTheme, NULL) : "Light");

    ffplist_free(root_node);
    return NULL;
}

static void printQuartzCompositor(FFinstance* instance) {
    FFstrbuf fileName;
    ffStrbufInitS(&fileName, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&fileName, "/Library/Preferences/.GlobalPreferences.plist");

    FFstrbuf content;
    ffStrbufInit(&content);
    ffAppendFileBuffer(fileName.chars, &content);
    ffStrbufDestroy(&fileName);
    if(content.length == 0)
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "reading Quartz Compositor preference file failed");
        ffStrbufDestroy(&content);
        return;
    }

    FFstrbuf theme;
    ffStrbufInit(&theme);
    const char* error = quartzCompositorParsePlist(instance, &content, &theme);
    ffStrbufDestroy(&content);

    if(error != NULL)
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "%s", error);
    else
        printWMTheme(instance, theme.chars);
}
#else
static void printQuartzCompositor(FFinstance* instance)
{
    FF_UNUSED(instance);
    ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "Fastfetch was compiled without libplist support");
}
#endif

void ffPrintWMTheme(FFinstance* instance)
{
    #ifdef __ANDROID__
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "WM theme detection is not supported on Android");
        return;
    #endif

    const FFDisplayServerResult* result = ffConnectDisplayServer(instance);

    if(result->wmPrettyName.length == 0)
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "WM Theme needs sucessfull WM detection");
        return;
    }

    if(ffStrbufIgnCaseCompS(&result->wmPrettyName, "KWin") == 0 || ffStrbufIgnCaseCompS(&result->wmPrettyName, "KDE") == 0 || ffStrbufIgnCaseCompS(&result->wmPrettyName, "Plasma") == 0)
        printWMThemeFromConfigFile(instance, "kwinrc", "theme =", "Breeze");
    else if(ffStrbufIgnCaseCompS(&result->wmPrettyName, "Xfwm4") == 0 || ffStrbufIgnCaseCompS(&result->wmPrettyName, "Xfwm") == 0)
        printXFWM4(instance);
    else if(ffStrbufIgnCaseCompS(&result->wmPrettyName, "Mutter") == 0)
    {
        if(ffStrbufIgnCaseCompS(&result->dePrettyName, "Gnome") == 0)
            printMutter(instance);
        else
            printGTKThemeAsWMTheme(instance);
    }
    else if(ffStrbufIgnCaseCompS(&result->wmPrettyName, "Muffin") == 0)
        printMuffin(instance);
    else if(ffStrbufIgnCaseCompS(&result->wmPrettyName, "Marco") == 0)
        printWMThemeFromSettings(instance, "/org/mate/Marco/general/theme", "org.mate.Marco.general", NULL, "theme");
    else if(ffStrbufIgnCaseCompS(&result->wmPrettyName, "Openbox") == 0)
        printOpenbox(instance, &result->dePrettyName);
    else if(ffStrbufIgnCaseCompS(&result->wmPrettyName, "Quartz Compositor") == 0)
        printQuartzCompositor(instance);
    else
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "Unknown WM: %s", result->wmPrettyName.chars);
}
