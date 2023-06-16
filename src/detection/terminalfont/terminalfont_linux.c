#include "terminalfont.h"
#include "common/settings.h"
#include "common/properties.h"
#include "common/parsing.h"
#include "detection/terminalshell/terminalshell.h"
#include "detection/displayserver/displayserver.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"

static const char* getSystemMonospaceFont(const FFinstance* instance)
{
    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseEqualS(&wmde->dePrettyName, "Cinnamon"))
    {
        const char* systemMonospaceFont = ffSettingsGet(instance, "/org/cinnamon/desktop/interface/monospace-font-name", "org.cinnamon.desktop.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(systemMonospaceFont))
            return systemMonospaceFont;
    }
    else if(ffStrbufIgnCaseEqualS(&wmde->dePrettyName, "Mate"))
    {
        const char* systemMonospaceFont = ffSettingsGet(instance, "/org/mate/interface/monospace-font-name", "org.mate.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(systemMonospaceFont))
            return systemMonospaceFont;
    }

    return ffSettingsGet(instance, "/org/gnome/desktop/interface/monospace-font-name", "org.gnome.desktop.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
}

static void detectFromGSettings(const FFinstance* instance, const char* profilePath, const char* profileList, const char* profile, const char* defaultProfileKey, FFTerminalFontResult* terminalFont)
{
    FF_AUTO_FREE const char* defaultProfile = ffSettingsGetGSettings(instance, profileList, NULL, defaultProfileKey, FF_VARIANT_TYPE_STRING).strValue;
    if(!ffStrSet(defaultProfile))
    {
        ffStrbufAppendF(&terminalFont->error, "Could not get default profile from gsettings: %s", profileList);
        return;
    }

    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateA(128);
    ffStrbufAppendS(&path, profilePath);
    ffStrbufAppendS(&path, defaultProfile);
    ffStrbufAppendC(&path, '/');

    if(!ffSettingsGetGSettings(instance, profile, path.chars, "use-system-font", FF_VARIANT_TYPE_BOOL).boolValue)
    {
        FF_AUTO_FREE const char* fontName = ffSettingsGetGSettings(instance, profile, path.chars, "font", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(fontName))
            ffFontInitPango(&terminalFont->font, fontName);
        else
            ffStrbufAppendF(&terminalFont->error, "Couldn't get terminal font from GSettings (%s::%s::font)", profile, path.chars);
    }
    else
    {
        const char* fontName = getSystemMonospaceFont(instance);
        if(ffStrSet(fontName))
            ffFontInitPango(&terminalFont->font, fontName);
        else
            ffStrbufAppendS(&terminalFont->error, "Could't get system monospace font name from GSettings / DConf");
    }
}

static void detectFromConfigFile(const FFinstance* instance, const char* configFile, const char* start, FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    ffParsePropFileConfig(instance, configFile, start, &fontName);

    if(fontName.length == 0)
        ffStrbufAppendF(&terminalFont->error, "Couldn't find %s in .config/%s", start, configFile);
    else
        ffFontInitPango(&terminalFont->font, fontName.chars);
}

static void detectKonsole(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY profile = ffStrbufCreate();
    ffParsePropFileConfig(instance, "konsolerc", "DefaultProfile =", &profile);

    if(profile.length == 0)
    {
        ffStrbufAppendS(&terminalFont->error, "Couldn't find \"DefaultProfile=%[^\\n]\" in \".config/konsolerc\"");
        return;
    }

    FF_STRBUF_AUTO_DESTROY profilePath = ffStrbufCreateA(32);
    ffStrbufAppendS(&profilePath, "konsole/");
    ffStrbufAppend(&profilePath, &profile);

    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    ffParsePropFileData(instance, profilePath.chars, "Font =", &fontName);

    if(fontName.length == 0)
        ffStrbufAppendF(&terminalFont->error, "Couldn't find \"Font=%%[^\\n]\" in \"%s\"", profilePath.chars);
    else
        ffFontInitQt(&terminalFont->font, fontName.chars);
}

static void detectXFCETerminal(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY useSysFont = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();

    ffParsePropFileConfigValues(instance, "xfce4/terminal/terminalrc", 2, (FFpropquery[]) {
        {"FontUseSystem = ", &useSysFont},
        {"FontName = ", &fontName}
    });

    if(useSysFont.length == 0 || ffStrbufIgnCaseCompS(&useSysFont, "false") == 0)
    {
        if(fontName.length == 0)
            ffStrbufAppendS(&terminalFont->error, "Couldn't find FontName in .config/xfce4/terminal/terminalrc");
        else
            ffFontInitPango(&terminalFont->font, fontName.chars);
    }
    else
    {
        const char* systemFontName = ffSettingsGetXFConf(instance, "xsettings", "/Gtk/MonospaceFontName", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(systemFontName))
            ffFontInitPango(&terminalFont->font, systemFontName);
        else
            ffStrbufAppendS(&terminalFont->error, "Couldn't find xsettings::/Gtk/MonospaceFontName in XFConf");
    }
}

static void detectDeepinTerminal(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY profile = ffStrbufCreate();

    ffStrbufAppend(&profile, &instance->state.platform.homeDir);
    ffStrbufAppendS(&profile, ".config/deepin/deepin-terminal/config.conf"); //TODO: Use config dirs
    FILE* file = fopen(profile.chars, "r");

    if(file)
    {
        char* line = NULL;
        size_t len = 0;

        for(int count = 0; getline(&line, &len, file) != -1 && count < 2;)
        {
            if(strcmp(line, "[basic.interface.font]\n") == 0)
            {
                if(getline(&line, &len, file) != -1)
                    ffParsePropLine(line, "value=", &fontName);
                ++count;
            }
            else if(strcmp(line, "[basic.interface.font_size]\n") == 0)
            {
                if(getline(&line, &len, file) != -1)
                    ffParsePropLine(line, "value=", &fontSize);
                ++count;
            }
        }

        free(line);
        fclose(file);
    }

    if(fontName.length == 0)
        ffStrbufAppendS(&fontName, "Noto Sans Mono");
    if(fontSize.length == 0)
        ffStrbufAppendS(&fontSize, "11");

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);
}

static void detectFootTerminal(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY font = ffStrbufCreate();

    if (!ffParsePropFileConfig(instance, "foot/foot.ini", "font=", &font) || !ffStrSet(font.chars))
    {
        ffFontInitValues(&terminalFont->font, "monospace", "8");
        return;
    }

    //Sarasa Term SC Nerd:size=8
    uint32_t colon = ffStrbufFirstIndexC(&font, ':');
    if(colon == font.length)
    {
        ffFontInitValues(&terminalFont->font, font.chars, "8");
        return;
    }
    uint32_t equal = ffStrbufNextIndexS(&font, colon, "size=");
    font.chars[colon] = 0;
    if (equal == font.length)
    {
        ffFontInitValues(&terminalFont->font, font.chars, "8");
        return;
    }
    ffFontInitValues(&terminalFont->font, font.chars, &font.chars[equal + strlen("size=")]);
}

static void detectQTerminal(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    ffParsePropFileConfigValues(instance, "qterminal.org/qterminal.ini", 2, (FFpropquery[]) {
        {"fontFamily=", &fontName},
        {"fontSize=", &fontSize},
    });

    if (fontName.length == 0)
        ffStrbufAppendS(&fontName, "monospace");
    if (fontSize.length == 0)
        ffStrbufAppendS(&fontSize, "12");
    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);
}

void ffDetectTerminalFontPlatform(const FFinstance* instance, const FFTerminalShellResult* terminalShell, FFTerminalFontResult* terminalFont)
{
    if(ffStrbufIgnCaseEqualS(&terminalShell->terminalProcessName, "konsole"))
        detectKonsole(instance, terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminalShell->terminalProcessName, "xfce4-terminal"))
        detectXFCETerminal(instance, terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminalShell->terminalProcessName, "lxterminal"))
        detectFromConfigFile(instance, "lxterminal/lxterminal.conf", "fontname =", terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminalShell->terminalProcessName, "tilix"))
        detectFromGSettings(instance, "/com/gexperts/Tilix/profiles/", "com.gexperts.Tilix.ProfilesList", "com.gexperts.Tilix.Profile", "default", terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminalShell->terminalProcessName, "gnome-terminal-"))
        detectFromGSettings(instance, "/org/gnome/terminal/legacy/profiles:/:", "org.gnome.Terminal.ProfilesList", "org.gnome.Terminal.Legacy.Profile", "default", terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminalShell->terminalProcessName, "mate-terminal"))
        detectFromGSettings(instance, "/org/mate/terminal/profiles/", "org.mate.terminal.global", "org.mate.terminal.profile", "default-profile", terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminalShell->terminalProcessName, "deepin-terminal"))
        detectDeepinTerminal(instance, terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminalShell->terminalProcessName, "foot"))
        detectFootTerminal(instance, terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminalShell->terminalProcessName, "qterminal"))
        detectQTerminal(instance, terminalFont);
}
