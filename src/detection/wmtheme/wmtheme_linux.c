#include "wmtheme.h"
#include "common/io/io.h"
#include "common/properties.h"
#include "common/parsing.h"
#include "common/settings.h"
#include "detection/gtk_qt/gtk_qt.h"
#include "detection/displayserver/displayserver.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"

static bool detectWMThemeFromConfigFile(const char* configFile, const char* themeRegex, const char* defaultValue, FFstrbuf* themeOrError)
{
    if(!ffParsePropFileConfig(configFile, themeRegex, themeOrError))
    {
        ffStrbufAppendF(themeOrError, "Config file %s doesn't exist", configFile);
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

static bool detectWMThemeFromSettings(const char* dconfKey, const char* gsettingsSchemaName, const char* gsettingsPath, const char* gsettingsKey, FFstrbuf* themeOrError)
{
    const char* theme = ffSettingsGet(dconfKey, gsettingsSchemaName, gsettingsPath, gsettingsKey, FF_VARIANT_TYPE_STRING).strValue;

    if(!ffStrSet(theme))
    {
        ffStrbufAppendS(themeOrError, "Couldn't find WM theme in DConf or GSettings");
        return false;
    }

    ffStrbufAppendS(themeOrError, theme);
    return true;
}

static bool detectGTKThemeAsWMTheme(FFstrbuf* themeOrError)
{
    const FFGTKResult* gtk = ffDetectGTK4();
    if(gtk->theme.length > 0)
        goto ok;

    gtk = ffDetectGTK3();
    if(gtk->theme.length > 0)
        goto ok;

    gtk = ffDetectGTK2();
    if(gtk->theme.length > 0)
        goto ok;

    ffStrbufAppendS(themeOrError, "Couldn't detect GTK4/3/2 theme");
    return false;

ok:
    ffStrbufAppend(themeOrError, &gtk->theme);
    return true;
}

static bool detectMutter(FFstrbuf* themeOrError)
{
    const char* theme = ffSettingsGet("/org/gnome/shell/extensions/user-theme/name", "org.gnome.shell.extensions.user-theme", NULL, "name", FF_VARIANT_TYPE_STRING).strValue;
    if(ffStrSet(theme))
    {
        ffStrbufAppendS(themeOrError, theme);
        return true;
    }

    return detectGTKThemeAsWMTheme(themeOrError);
}

static bool detectMuffin(FFstrbuf* themeOrError)
{
    FF_AUTO_FREE const char* name = ffSettingsGet("/org/cinnamon/theme/name", "org.cinnamon.theme", NULL, "name", FF_VARIANT_TYPE_STRING).strValue;
    FF_AUTO_FREE const char* theme = ffSettingsGet("/org/cinnamon/desktop/wm/preferences/theme", "org.cinnamon.desktop.wm.preferences", NULL, "theme", FF_VARIANT_TYPE_STRING).strValue;

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
        ffStrbufAppendS(themeOrError, name);
        return true;
    }

    ffStrbufAppendF(themeOrError, "%s (%s)", name, theme);
    return true;
}

static bool detectXFWM4(FFstrbuf* themeOrError)
{
    const char* theme = ffSettingsGetXFConf("xfwm4", "/general/theme", FF_VARIANT_TYPE_STRING).strValue;

    if(theme == NULL)
    {
        ffStrbufAppendS(themeOrError, "Couldn't find xfwm4::/general/theme in XFConf");
        return false;
    }

    ffStrbufAppendS(themeOrError, theme);
    return true;
}

static bool detectOpenbox(const FFstrbuf* dePrettyName, FFstrbuf* themeOrError)
{
    FF_STRBUF_AUTO_DESTROY absolutePath = ffStrbufCreateA(64);
    const char *configFileSubpath = "openbox/rc.xml";
    if (ffStrbufIgnCaseCompS(dePrettyName, "LXQt") == 0)
        configFileSubpath = "openbox/lxqt-rc.xml";
    else if (ffStrbufIgnCaseCompS(dePrettyName, "LXDE") == 0)
        configFileSubpath = "openbox/lxde-rc.xml";

    if (!ffSearchUserConfigFile(&instance.state.platform.configDirs, configFileSubpath, &absolutePath))
    {
        ffStrbufAppendF(themeOrError, "Couldn't find config file \"%s\"", configFileSubpath);
        return false;
    }

    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    if (!ffReadFileBuffer(absolutePath.chars, &content))
    {
        ffStrbufAppendF(themeOrError, "Couldn't read \"%s\"", absolutePath.chars);
        return false;
    }

    const char *themeStart = strstr(content.chars, "<theme>");
    if (themeStart == NULL)
        goto theme_not_found;

    const char *themeEnd = strstr(themeStart, "</theme>");
    if (__builtin_expect(themeEnd == NULL, false)) // very rare case
        goto theme_not_found;

    const char *nameStart = strstr(themeStart, "<name>");
    if (nameStart == NULL)
        goto name_not_found;

    const char *nameEnd = strstr(nameStart, "</name>");
    if (nameEnd == NULL || nameEnd > themeEnd) // (nameEnd > themeEnd) means name is not a theme's child
        goto name_not_found;

    nameStart += strlen("<name>");
    ffStrbufAppendNS(themeOrError, (uint32_t)(nameEnd - nameStart), nameStart);
    ffStrbufTrim(themeOrError, ' ');

    if(themeOrError->length == 0)
        goto name_not_found;

    return true;

theme_not_found:
    ffStrbufAppendF(themeOrError, "Couldn't find theme node in \"%s\"", absolutePath.chars);
    return false;

name_not_found:
    ffStrbufAppendF(themeOrError, "Couldn't find theme name in \"%s\"", absolutePath.chars);
    return false;
}

bool ffDetectWmTheme(FFstrbuf* themeOrError)
{
    const FFDisplayServerResult* wm = ffConnectDisplayServer();

    if(wm->wmPrettyName.length == 0)
    {
        ffStrbufAppendS(themeOrError, "WM Theme needs successful WM detection");
        return false;
    }

    if(ffStrbufIgnCaseCompS(&wm->wmPrettyName, FF_WM_PRETTY_KWIN) == 0)
        return detectWMThemeFromConfigFile("kwinrc", "theme =", "Breeze", themeOrError);

    if(ffStrbufIgnCaseCompS(&wm->wmPrettyName, FF_WM_PRETTY_XFWM4) == 0)
        return detectXFWM4(themeOrError);

    if(ffStrbufIgnCaseCompS(&wm->wmPrettyName, FF_WM_PRETTY_MUTTER) == 0)
    {
        if(
            ffStrbufIgnCaseCompS(&wm->dePrettyName, FF_DE_PRETTY_GNOME) == 0 ||
            ffStrbufIgnCaseEqualS(&wm->dePrettyName, FF_DE_PRETTY_GNOME_CLASSIC)
        )
            return detectMutter(themeOrError);
        else
            return detectGTKThemeAsWMTheme(themeOrError);
    }

    if(ffStrbufIgnCaseCompS(&wm->wmPrettyName, FF_WM_PRETTY_MUFFIN) == 0)
        return detectMuffin(themeOrError);

    if(ffStrbufIgnCaseCompS(&wm->wmPrettyName, FF_WM_PRETTY_MARCO) == 0)
        return detectWMThemeFromSettings("/org/mate/Marco/general/theme", "org.mate.Marco.general", NULL, "theme", themeOrError);

    if(ffStrbufIgnCaseCompS(&wm->wmPrettyName, FF_WM_PRETTY_OPENBOX) == 0)
        return detectOpenbox(&wm->dePrettyName, themeOrError);

    ffStrbufAppendS(themeOrError, "Unknown WM: ");
    ffStrbufAppend(themeOrError, &wm->wmPrettyName);
    return false;
}
