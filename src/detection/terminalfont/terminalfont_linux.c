#include "terminalfont.h"
#include "common/settings.h"
#include "common/properties.h"
#include "common/parsing.h"
#include "detection/terminalshell/terminalshell.h"
#include "detection/displayserver/displayserver.h"
#include "util/stringUtils.h"

static const char* getSystemMonospaceFont(const FFinstance* instance)
{
    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, "Cinnamon") == 0)
    {
        const char* systemMonospaceFont = ffSettingsGet(instance, "/org/cinnamon/desktop/interface/monospace-font-name", "org.cinnamon.desktop.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(systemMonospaceFont))
            return systemMonospaceFont;
    }
    else if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, "Mate") == 0)
    {
        const char* systemMonospaceFont = ffSettingsGet(instance, "/org/mate/interface/monospace-font-name", "org.mate.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(systemMonospaceFont))
            return systemMonospaceFont;
    }

    return ffSettingsGet(instance, "/org/gnome/desktop/interface/monospace-font-name", "org.gnome.desktop.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
}

static void detectFromGSettings(const FFinstance* instance, char* profilePath, char* profileList, char* profile, FFTerminalFontResult* terminalFont)
{
    const char* defaultProfile = ffSettingsGetGSettings(instance, profileList, NULL, "default", FF_VARIANT_TYPE_STRING).strValue;
    if(!ffStrSet(defaultProfile))
    {
        ffStrbufAppendF(&terminalFont->error, "Could not get default profile from gsettings: %s", profileList);
        return;
    }

    FFstrbuf path;
    ffStrbufInitA(&path, 128);
    ffStrbufAppendS(&path, profilePath);
    ffStrbufAppendS(&path, defaultProfile);
    ffStrbufAppendC(&path, '/');

    if(!ffSettingsGetGSettings(instance, profile, path.chars, "use-system-font", FF_VARIANT_TYPE_BOOL).boolValue)
    {
        const char* fontName = ffSettingsGetGSettings(instance, profile, path.chars, "font", FF_VARIANT_TYPE_STRING).strValue;
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

    ffStrbufDestroy(&path);
}

static void detectFromConfigFile(const FFinstance* instance, const char* configFile, const char* start, FFTerminalFontResult* terminalFont)
{
    FFstrbuf fontName;
    ffStrbufInit(&fontName);
    ffParsePropFileConfig(instance, configFile, start, &fontName);

    if(fontName.length == 0)
        ffStrbufAppendF(&terminalFont->error, "Couldn't find %s in .config/%s", start, configFile);
    else
        ffFontInitPango(&terminalFont->font, fontName.chars);

    ffStrbufDestroy(&fontName);
}

static void detectKonsole(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FFstrbuf profile;
    ffStrbufInit(&profile);
    ffParsePropFileConfig(instance, "konsolerc", "DefaultProfile =", &profile);

    if(profile.length == 0)
    {
        ffStrbufAppendS(&terminalFont->error, "Couldn't find \"DefaultProfile=%[^\\n]\" in \".config/konsolerc\"");
        ffStrbufDestroy(&profile);
        return;
    }

    FFstrbuf profilePath;
    ffStrbufInitA(&profilePath, 32);
    ffStrbufAppendS(&profilePath, "konsole/");
    ffStrbufAppend(&profilePath, &profile);

    ffStrbufDestroy(&profile);

    FFstrbuf fontName;
    ffStrbufInit(&fontName);
    ffParsePropFileData(instance, profilePath.chars, "Font =", &fontName);

    if(fontName.length == 0)
        ffStrbufAppendF(&terminalFont->error, "Couldn't find \"Font=%%[^\\n]\" in \"%s\"", profilePath.chars);
    else
        ffFontInitQt(&terminalFont->font, fontName.chars);

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&profilePath);
}

static void detectXFCETerminal(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FFstrbuf useSysFont;
    ffStrbufInit(&useSysFont);

    FFstrbuf fontName;
    ffStrbufInit(&fontName);

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

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&useSysFont);
}

static void detectDeepinTerminal(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FFstrbuf fontName;
    ffStrbufInit(&fontName);

    FFstrbuf fontSize;
    ffStrbufInit(&fontSize);

    FFstrbuf profile;
    ffStrbufInit(&profile);
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

    ffStrbufDestroy(&profile);

    if(fontName.length == 0)
        ffStrbufAppendS(&fontName, "Noto Sans Mono");
    if(fontSize.length == 0)
        ffStrbufAppendS(&fontSize, "11");

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&fontSize);
}

static void detectFootTerminal(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY font;
    ffStrbufInit(&font);

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

void ffDetectTerminalFontPlatform(const FFinstance* instance, const FFTerminalShellResult* terminalShell, FFTerminalFontResult* terminalFont)
{
    if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "konsole") == 0)
        detectKonsole(instance, terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "xfce4-terminal") == 0)
        detectXFCETerminal(instance, terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "lxterminal") == 0)
        detectFromConfigFile(instance, "lxterminal/lxterminal.conf", "fontname =", terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "tilix") == 0)
        detectFromGSettings(instance, "/com/gexperts/Tilix/profiles/", "com.gexperts.Tilix.ProfilesList", "com.gexperts.Tilix.Profile", terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "gnome-terminal-") == 0)
        detectFromGSettings(instance, "/org/gnome/terminal/legacy/profiles:/:", "org.gnome.Terminal.ProfilesList", "org.gnome.Terminal.Legacy.Profile", terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "deepin-terminal") == 0)
        detectDeepinTerminal(instance, terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "foot") == 0)
        detectFootTerminal(instance, terminalFont);
}
