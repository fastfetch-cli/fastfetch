#include "fastfetch.h"
#include "common/properties.h"
#include "common/font.h"
#include "common/settings.h"
#include "detection/displayserver/displayserver.h"
#include "terminalfont.h"

static const char* getSystemMonospaceFont(FFinstance* instance)
{
    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, "Cinnamon") == 0)
    {
        const char* systemMonospaceFont = ffSettingsGet(instance, "/org/cinnamon/desktop/interface/monospace-font-name", "org.cinnamon.desktop.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
        if(systemMonospaceFont != NULL)
            return systemMonospaceFont;
    }
    else if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, "Mate") == 0)
    {
        const char* systemMonospaceFont = ffSettingsGet(instance, "/org/mate/interface/monospace-font-name", "org.mate.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
        if(systemMonospaceFont != NULL)
            return systemMonospaceFont;
    }

    return ffSettingsGet(instance, "/org/gnome/desktop/interface/monospace-font-name", "org.gnome.desktop.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
}

static const char* detectTerminalFontFromConfigFile(FFinstance* instance, const char* configFile, const char* start, FFfont* font)
{
    FFstrbuf fontName;
    ffStrbufInit(&fontName);
    ffParsePropFileConfig(instance, configFile, start, &fontName);

    if(fontName.length == 0)
    {
        ffStrbufDestroy(&fontName);
        return "Couldn't find terminal font in \"$XDG_CONFIG_HOME\"";
    }

    ffFontInitPango(font, fontName.chars);
    ffStrbufDestroy(&fontName);
    return NULL;
}

static const char* detectTerminalFontFromGSettings(FFinstance* instance, char* profilePath, char* profileList, char* profile, FFfont* font)
{
    const char* defaultProfile = ffSettingsGetGSettings(instance, profileList, NULL, "default", FF_VARIANT_TYPE_STRING).strValue;
    if(defaultProfile == NULL)
        return "Couldn't get \"default\" profile from gsettings";

    FFstrbuf path;
    ffStrbufInitA(&path, 128);
    ffStrbufAppendS(&path, profilePath);
    ffStrbufAppendS(&path, defaultProfile);
    ffStrbufAppendC(&path, '/');

    const char* fontName;

    if(!ffSettingsGetGSettings(instance, profile, path.chars, "use-system-font", FF_VARIANT_TYPE_BOOL).boolValue) // custom font
    {
        fontName = ffSettingsGetGSettings(instance, profile, path.chars, "font", FF_VARIANT_TYPE_STRING).strValue;
        if(fontName == NULL)
        {
            ffStrbufDestroy(&path);
            return "Couldn't get terminal font from GSettings";
        }
    }
    else // system font
    {
        fontName = getSystemMonospaceFont(instance);
        if(fontName == NULL)
        {
            ffStrbufDestroy(&path);
            return "Could't get system monospace font name from GSettings / DConf";
        }
    }

    ffFontInitPango(font, fontName);
    return NULL;
}

static const char* detectKonsole(FFinstance* instance, FFfont* font)
{
    FFstrbuf profile;
    ffStrbufInit(&profile);
    ffParsePropFileConfig(instance, "konsolerc", "DefaultProfile =", &profile);

    if(profile.length == 0)
    {
        ffStrbufDestroy(&profile);
        return "Couldn't find \"DefaultProfile=%[^\\n]\" in \".config/konsolerc\"";
    }

    FFstrbuf profilePath;
    ffStrbufInitA(&profilePath, 64);
    ffStrbufAppendS(&profilePath, ".local/share/konsole/");
    ffStrbufAppend(&profilePath, &profile);

    FFstrbuf fontName;
    ffStrbufInit(&fontName);
    ffParsePropFileHome(instance, profilePath.chars, "Font =", &fontName);

    ffStrbufDestroy(&profilePath);
    ffStrbufDestroy(&profile);

    if(fontName.length == 0)
    {
        ffStrbufDestroy(&fontName);
        return "Couldn't find \"Font=%[^\\n]\"";
    }

    ffFontInitQt(font, fontName.chars);
    ffStrbufDestroy(&fontName);

    return NULL;
}

static const char* detectXCFETerminal(FFinstance* instance, FFfont* font)
{
    FFstrbuf useSysFont;
    ffStrbufInit(&useSysFont);

    if(!ffParsePropFileConfig(instance, "xfce4/terminal/terminalrc", "FontUseSystem =", &useSysFont))
    {
        ffStrbufDestroy(&useSysFont);
        return "Couldn't open \"$XDG_CONFIG_HOME/xfce4/terminal/terminalrc\"";
    }

    if(useSysFont.length == 0 || ffStrbufIgnCaseCompS(&useSysFont, "FALSE") == 0)
    {
        ffStrbufDestroy(&useSysFont);
        return detectTerminalFontFromConfigFile(instance, "xfce4/terminal/terminalrc", "FontName =", font);
    }

    ffStrbufDestroy(&useSysFont);

    const char* fontName = ffSettingsGetXFConf(instance, "xsettings", "/Gtk/MonospaceFontName", FF_VARIANT_TYPE_STRING).strValue;

    if(fontName == NULL)
        return "Couldn't find \"xsettings::/Gtk/MonospaceFontName\" in XFConf";

    ffFontInitPango(font, fontName);

    return NULL;
}

const char* ffDetectTerminalFontPlatform(FFinstance* instance, const FFTerminalShellResult* shellInfo, FFfont* font)
{
    if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "konsole") == 0)
        return detectKonsole(instance, font);

    if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "xfce4-terminal") == 0)
        return detectXCFETerminal(instance, font);

    if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "lxterminal") == 0)
        return detectTerminalFontFromConfigFile(instance, "lxterminal/lxterminal.conf", "fontname =", font);

    if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "tilix") == 0)
        return detectTerminalFontFromGSettings(instance, "/com/gexperts/Tilix/profiles/", "com.gexperts.Tilix.ProfilesList", "com.gexperts.Tilix.Profile", font);

    if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "gnome-terminal-") == 0)
        return detectTerminalFontFromGSettings(instance, "/org/gnome/terminal/legacy/profiles:/:", "org.gnome.Terminal.ProfilesList", "org.gnome.Terminal.Legacy.Profile", font);

    return "";
}
