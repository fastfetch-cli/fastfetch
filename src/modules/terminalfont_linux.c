#include "fastfetch.h"
#include "common/properties.h"
#include "common/printing.h"
#include "common/font.h"
#include "common/settings.h"
#include "detection/displayserver/displayserver.h"
#include "detection/terminalshell.h"
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

static void printTerminalFontFromConfigFile(FFinstance* instance, const char* configFile, const char* start)
{
    FFstrbuf fontName;
    ffStrbufInit(&fontName);
    ffParsePropFileConfig(instance, configFile, start, &fontName);

    if(fontName.length == 0)
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Couldn't find terminal font in \"$XDG_CONFIG_HOME/%s\"", configFile);
    else
    {
        FFfont font;
        ffFontInitPango(&font, fontName.chars);
        ffPrintTerminalFontResult(instance, fontName.chars, &font);
        ffFontDestroy(&font);
    }

    ffStrbufDestroy(&fontName);
}

static void printTerminalFontFromGSettings(FFinstance* instance, char* profilePath, char* profileList, char* profile)
{
    const char* defaultProfile = ffSettingsGetGSettings(instance, profileList, NULL, "default", FF_VARIANT_TYPE_STRING).strValue;
    if(defaultProfile == NULL)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Couldn't get \"default\" profile from gsettings");
        return;
    }

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
            ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Couldn't get terminal font from GSettings (%s::%s::font)", profile, path.chars);
    }
    else // system font
    {
        fontName = getSystemMonospaceFont(instance);
        if(fontName == NULL)
            ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Could't get system monospace font name from GSettings / DConf");
    }

    ffStrbufDestroy(&path);

    if(fontName == NULL)
        return;

    FFfont font;
    ffFontInitPango(&font, fontName);
    ffPrintTerminalFontResult(instance, fontName, &font);
    ffFontDestroy(&font);
}

static void printKonsole(FFinstance* instance)
{
    FFstrbuf profile;
    ffStrbufInit(&profile);
    ffParsePropFileConfig(instance, "konsolerc", "DefaultProfile =", &profile);

    if(profile.length == 0)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "%s", "Couldn't find \"DefaultProfile=%[^\\n]\" in \".config/konsolerc\"");
        ffStrbufDestroy(&profile);
        return;
    }

    FFstrbuf profilePath;
    ffStrbufInitA(&profilePath, 64);
    ffStrbufAppendS(&profilePath, ".local/share/konsole/");
    ffStrbufAppend(&profilePath, &profile);

    FFstrbuf fontName;
    ffStrbufInit(&fontName);
    ffParsePropFileHome(instance, profilePath.chars, "Font =", &fontName);

    if(fontName.length == 0)
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Couldn't find \"Font=%%[^\\n]\" in \"%s\"", profilePath.chars);
    else
    {
        FFfont font;
        ffFontInitQt(&font, fontName.chars);
        ffPrintTerminalFontResult(instance, fontName.chars, &font);
        ffFontDestroy(&font);
    }

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&profilePath);
    ffStrbufDestroy(&profile);
}

static void printXCFETerminal(FFinstance* instance)
{
    FFstrbuf useSysFont;
    ffStrbufInit(&useSysFont);

    if(!ffParsePropFileConfig(instance, "xfce4/terminal/terminalrc", "FontUseSystem =", &useSysFont))
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Couldn't open \"$XDG_CONFIG_HOME/xfce4/terminal/terminalrc\"");
        ffStrbufDestroy(&useSysFont);
        return;
    }

    if(useSysFont.length == 0 || ffStrbufIgnCaseCompS(&useSysFont, "FALSE") == 0)
    {
        printTerminalFontFromConfigFile(instance, "xfce4/terminal/terminalrc", "FontName =");
        ffStrbufDestroy(&useSysFont);
        return;
    }

    ffStrbufDestroy(&useSysFont);

    const char* fontName = ffSettingsGetXFConf(instance, "xsettings", "/Gtk/MonospaceFontName", FF_VARIANT_TYPE_STRING).strValue;

    if(fontName == NULL)
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Couldn't find \"xsettings::/Gtk/MonospaceFontName\" in XFConf");
    else
    {
        FFfont font;
        ffFontInitPango(&font, fontName);
        ffPrintTerminalFontResult(instance, fontName, &font);
        ffFontDestroy(&font);
    }
}

bool ffPrintTerminalFontPlatform(FFinstance* instance, const FFTerminalShellResult* shellInfo)
{
    bool success = true;
    if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "konsole") == 0)
        printKonsole(instance);
    else if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "xfce4-terminal") == 0)
        printXCFETerminal(instance);
    else if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "lxterminal") == 0)
        printTerminalFontFromConfigFile(instance, "lxterminal/lxterminal.conf", "fontname =");
    else if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "tilix") == 0)
        printTerminalFontFromGSettings(instance, "/com/gexperts/Tilix/profiles/", "com.gexperts.Tilix.ProfilesList", "com.gexperts.Tilix.Profile");
    else if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "gnome-terminal-") == 0)
        printTerminalFontFromGSettings(instance, "/org/gnome/terminal/legacy/profiles:/:", "org.gnome.Terminal.ProfilesList", "org.gnome.Terminal.Legacy.Profile");
    else
        success = false;
    return success;
}
