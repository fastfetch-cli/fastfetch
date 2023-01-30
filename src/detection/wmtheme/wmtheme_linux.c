#include "fastfetch.h"
#include "common/properties.h"
#include "common/parsing.h"
#include "common/settings.h"
#include "detection/gtk.h"
#include "detection/displayserver/displayserver.h"
#include "wmtheme.h"

static bool detectWMThemeFromConfigFile(FFinstance* instance, const char* configFile, const char* themeRegex, const char* defaultValue, FFstrbuf* themeOrError)
{
    if(!ffParsePropFileConfig(instance, configFile, themeRegex, themeOrError))
    {
        ffStrbufInitF(themeOrError, "Config file %s doesn't exist", configFile);
        return false;
    }

    if(themeOrError->length == 0)
    {
        if(defaultValue == NULL)
        {
            ffStrbufAppendF(themeOrError, "Couldn't find WM theme in %s", configFile);
            return false;
        }

        ffStrbufAppendS(themeOrError, defaultValue);
        return true;
    }

    // Remove Plasma-generated prefixes
    uint32_t idx = 0;

    idx = ffStrbufFirstIndexS(themeOrError, "qml_");
    if(idx != themeOrError->length)
        ffStrbufSubstrAfter(themeOrError, idx + 3);

    idx = ffStrbufFirstIndexS(themeOrError, "svg__");
    if(idx != themeOrError->length)
        ffStrbufSubstrAfter(themeOrError, idx + 4);

    return true;
}

static bool detectWMThemeFromSettings(FFinstance* instance, const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey, FFstrbuf* themeOrError)
{
    const char* theme = ffSettingsGet(instance, dconfKey, gsettingsSchemaName, gsettingsPath, gsettingsKey, FF_VARIANT_TYPE_STRING).strValue;

    if(!ffStrSet(theme))
    {
        ffStrbufAppendS(themeOrError, "Couldn't find WM theme in DConf or GSettings");
        return false;
    }

    ffStrbufAppendS(themeOrError, theme);
    return true;
}

static bool detectGTKThemeAsWMTheme(FFinstance* instance, FFstrbuf* themeOrError)
{
    const FFGTKResult* gtk = ffDetectGTK4(instance);
    if(gtk->theme.length > 0)
        goto ok;

    gtk = ffDetectGTK3(instance);
    if(gtk->theme.length > 0)
        goto ok;

    gtk = ffDetectGTK2(instance);
    if(gtk->theme.length > 0)
        goto ok;

    ffStrbufAppendS(themeOrError, "Couldn't detect GTK4/3/2 theme");
    return false;

ok:
    ffStrbufAppend(themeOrError, &gtk->theme);
    return true;
}

static bool detectMutter(FFinstance* instance, FFstrbuf* themeOrError)
{
    const char* theme = ffSettingsGet(instance, "/org/gnome/shell/extensions/user-theme/name", "org.gnome.shell.extensions.user-theme", NULL, "name", FF_VARIANT_TYPE_STRING).strValue;
    if(ffStrSet(theme))
    {
        ffStrbufAppendS(themeOrError, theme);
        return true;
    }

    return detectGTKThemeAsWMTheme(instance, themeOrError);
}

static bool detectMuffin(FFinstance* instance, FFstrbuf* themeOrError)
{
    const char* name = ffSettingsGet(instance, "/org/cinnamon/theme/name", "org.cinnamon.theme", NULL, "name", FF_VARIANT_TYPE_STRING).strValue;
    const char* theme = ffSettingsGet(instance, "/org/cinnamon/desktop/wm/preferences/theme", "org.cinnamon.desktop.wm.preferences", NULL, "theme", FF_VARIANT_TYPE_STRING).strValue;

    if(name == NULL && theme == NULL)
    {
        ffStrbufAppendS(themeOrError, "Couldn't find muffin theme in GSettings / DConf");
        return false;
    }

    if(name == NULL)
    {
        ffStrbufAppendS(themeOrError, theme);
        return true;
    }
    if(theme == NULL)
    {
        ffStrbufAppendS(themeOrError, theme);
        return true;
    }
    ffStrbufAppendF(themeOrError, "%s (%s)", name, theme);
    return true;
}

static bool detectXFWM4(FFinstance* instance, FFstrbuf* themeOrError)
{
    const char* theme = ffSettingsGetXFConf(instance, "xfwm4", "/general/theme", FF_VARIANT_TYPE_STRING).strValue;

    if(theme == NULL)
    {
        ffStrbufAppendS(themeOrError, "Couldn't find xfwm4::/general/theme in XFConf");
        return false;
    }

    ffStrbufAppendS(themeOrError, theme);
    return true;
}

static bool detectOpenbox(FFinstance* instance, const FFstrbuf* dePrettyName, FFstrbuf* themeOrError)
{
    FFstrbuf absolutePath;
    ffStrbufInitA(&absolutePath, 64);
    ffStrbufAppend(&absolutePath, &instance->state.platform.homeDir);

    //TODO: use config dirs
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
        ffStrbufAppendF(themeOrError, "Couldn't open \"%s\"", absolutePath.chars);
        ffStrbufDestroy(&absolutePath);

        return false;
    }

    while(getline(&line, &len, file) != -1)
    {
        if(strstr(line, "<theme>") != 0)
            break;
    }

    while(getline(&line, &len, file) != -1)
    {
        if(strstr(line, "<name>") != 0)
        {
            ffStrbufAppendS(themeOrError, line);
            ffStrbufRemoveStrings(themeOrError, 2, "<name>", "</name>");
            ffStrbufTrimRight(themeOrError, '\n');
            ffStrbufTrim(themeOrError, ' ');
            break;
        }

        if(strstr(line, "</theme>") != 0) // sanity check
            break;
    }

    if(line != NULL)
        free(line);

    fclose(file);

    if(themeOrError->length == 0)
    {
        ffStrbufAppendF(themeOrError, "Couldn't find theme name in \"%s\"", absolutePath.chars);
        ffStrbufDestroy(&absolutePath);
        return false;
    }

    ffStrbufDestroy(&absolutePath);
    return true;
}

bool ffDetectWmTheme(FFinstance* instance, FFstrbuf* themeOrError)
{
    const FFDisplayServerResult* wm = ffConnectDisplayServer(instance);

    if(wm->wmPrettyName.length == 0)
    {
        ffStrbufAppendS(themeOrError, "WM Theme needs sucessfull WM detection");
        return false;
    }

    if(ffStrbufIgnCaseCompS(&wm->wmPrettyName, FF_WM_PRETTY_KWIN) == 0)
        return detectWMThemeFromConfigFile(instance, "kwinrc", "theme =", "Breeze", themeOrError);

    if(ffStrbufIgnCaseCompS(&wm->wmPrettyName, FF_WM_PRETTY_XFWM4) == 0)
        return detectXFWM4(instance, themeOrError);

    if(ffStrbufIgnCaseCompS(&wm->wmPrettyName, FF_WM_PRETTY_MUTTER) == 0)
    {
        if(ffStrbufIgnCaseCompS(&wm->dePrettyName, FF_DE_PRETTY_GNOME) == 0)
            return detectMutter(instance, themeOrError);
        else
            return detectGTKThemeAsWMTheme(instance, themeOrError);
    }

    if(ffStrbufIgnCaseCompS(&wm->wmPrettyName, FF_WM_PRETTY_MUFFIN) == 0)
        return detectMuffin(instance, themeOrError);

    if(ffStrbufIgnCaseCompS(&wm->wmPrettyName, FF_WM_PRETTY_MARCO) == 0)
        return detectWMThemeFromSettings(instance, "/org/mate/Marco/general/theme", "org.mate.Marco.general", NULL, "theme", themeOrError);

    if(ffStrbufIgnCaseCompS(&wm->wmPrettyName, FF_WM_PRETTY_OPENBOX) == 0)
        return detectOpenbox(instance, &wm->dePrettyName, themeOrError);

    ffStrbufAppendS(themeOrError, "Unknown WM: ");
    ffStrbufAppend(themeOrError, &wm->wmPrettyName);
    return false;
}
