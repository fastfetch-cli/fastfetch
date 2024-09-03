#include "terminalfont.h"
#include "common/settings.h"
#include "common/properties.h"
#include "common/parsing.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "detection/terminalshell/terminalshell.h"
#include "detection/displayserver/displayserver.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"
#include "util/binary.h"

static const char* getSystemMonospaceFont(void)
{
    const FFDisplayServerResult* wmde = ffConnectDisplayServer();

    if(ffStrbufIgnCaseEqualS(&wmde->dePrettyName, "Cinnamon"))
    {
        const char* systemMonospaceFont = ffSettingsGet("/org/cinnamon/desktop/interface/monospace-font-name", "org.cinnamon.desktop.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(systemMonospaceFont))
            return systemMonospaceFont;
    }
    else if(ffStrbufIgnCaseEqualS(&wmde->dePrettyName, "Mate"))
    {
        const char* systemMonospaceFont = ffSettingsGet("/org/mate/interface/monospace-font-name", "org.mate.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(systemMonospaceFont))
            return systemMonospaceFont;
    }

    return ffSettingsGet("/org/gnome/desktop/interface/monospace-font-name", "org.gnome.desktop.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
}

static void detectKgx(FFTerminalFontResult* terminalFont)
{
    // kgx (gnome console) doesn't support profiles
    if(!ffSettingsGet("/org/gnome/Console/use-system-font", "org.gnome.Console", NULL, "use-system-font", FF_VARIANT_TYPE_BOOL).boolValue)
    {
        FF_AUTO_FREE const char* fontName = ffSettingsGet("/org/gnome/Console/custom-font", "org.gnome.Console", NULL, "custom-font", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(fontName))
            ffFontInitPango(&terminalFont->font, fontName);
        else
            ffStrbufAppendF(&terminalFont->error, "Couldn't get terminal font from GSettings (org.gnome.Console::custom-font)");
    }
    else
    {
        FF_AUTO_FREE const char* fontName = getSystemMonospaceFont();
        if(ffStrSet(fontName))
            ffFontInitPango(&terminalFont->font, fontName);
        else
            ffStrbufAppendS(&terminalFont->error, "Couldn't get system monospace font name from GSettings / DConf");
    }
}

static void detectPtyxis(FFTerminalFontResult* terminalFont)
{
    if(!ffSettingsGet("/org/gnome/Ptyxis/use-system-font", "org.gnome.Ptyxis", NULL, "use-system-font", FF_VARIANT_TYPE_BOOL).boolValue)
    {
        FF_AUTO_FREE const char* fontName = ffSettingsGet("/org/gnome/Ptyxis/font-name", "org.gnome.Ptyxis", NULL, "font-name", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(fontName))
            ffFontInitPango(&terminalFont->font, fontName);
        else
            ffStrbufAppendF(&terminalFont->error, "Couldn't get terminal font from GSettings (org.gnome.Ptyxis::font-name)");
    }
    else
    {
        FF_AUTO_FREE const char* fontName = getSystemMonospaceFont();
        if(ffStrSet(fontName))
            ffFontInitPango(&terminalFont->font, fontName);
        else
            ffStrbufAppendS(&terminalFont->error, "Couldn't get system monospace font name from GSettings / DConf");
    }
}

static void detectFromGSettings(const char* profilePath, const char* profileList, const char* profile, const char* defaultProfileKey, FFTerminalFontResult* terminalFont)
{
    FF_AUTO_FREE const char* defaultProfile = ffSettingsGetGSettings(profileList, NULL, defaultProfileKey, FF_VARIANT_TYPE_STRING).strValue;
    if(!ffStrSet(defaultProfile))
    {
        ffStrbufAppendF(&terminalFont->error, "Could not get default profile from gsettings: %s", profileList);
        return;
    }

    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateA(128);
    ffStrbufAppendS(&path, profilePath);
    ffStrbufAppendS(&path, defaultProfile);
    ffStrbufAppendC(&path, '/');

    if(!ffSettingsGetGSettings(profile, path.chars, "use-system-font", FF_VARIANT_TYPE_BOOL).boolValue)
    {
        FF_AUTO_FREE const char* fontName = ffSettingsGetGSettings(profile, path.chars, "font", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(fontName))
            ffFontInitPango(&terminalFont->font, fontName);
        else
            ffStrbufAppendF(&terminalFont->error, "Couldn't get terminal font from GSettings (%s::%s::font)", profile, path.chars);
    }
    else
    {
        FF_AUTO_FREE const char* fontName = getSystemMonospaceFont();
        if(ffStrSet(fontName))
            ffFontInitPango(&terminalFont->font, fontName);
        else
            ffStrbufAppendS(&terminalFont->error, "Couldn't get system monospace font name from GSettings / DConf");
    }
}

static void detectFromConfigFile(const char* configFile, const char* start, FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    ffParsePropFileConfig(configFile, start, &fontName);

    if(fontName.length == 0)
        ffStrbufAppendF(&terminalFont->error, "Couldn't find %s in .config/%s", start, configFile);
    else
        ffFontInitPango(&terminalFont->font, fontName.chars);
}

static void detectKonsole(FFTerminalFontResult* terminalFont, const char* rcFile)
{
    FF_STRBUF_AUTO_DESTROY profile = ffStrbufCreate();
    if(!ffParsePropFileConfig(rcFile, "DefaultProfile =", &profile))
    {
        ffStrbufAppendF(&terminalFont->error, "Configuration \".config/%s\" doesn't exist", rcFile);
        return;
    }

    if(profile.length == 0)
    {
        ffStrbufAppendS(&terminalFont->error, "Built-in profile is used");
        return;
    }

    FF_STRBUF_AUTO_DESTROY profilePath = ffStrbufCreateA(32);
    ffStrbufAppendS(&profilePath, "konsole/");
    ffStrbufAppend(&profilePath, &profile);

    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    ffParsePropFileData(profilePath.chars, "Font =", &fontName);

    if(fontName.length == 0)
        ffStrbufAppendF(&terminalFont->error, "Couldn't find \"Font=%%[^\\n]\" in \"%s\"", profilePath.chars);
    else
        ffFontInitQt(&terminalFont->font, fontName.chars);
}

static void detectXFCETerminal(FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY useSysFont = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();

    const char* path = "xfce4/xfconf/xfce-perchannel-xml/xfce4-terminal.xml";
    bool configFound = ffParsePropFileConfigValues(path, 2, (FFpropquery[]) {
        {"<property name=\"font-use-system\" type=\"bool\" value=\"", &useSysFont},
        {"<property name=\"font-name\" type=\"string\" value=\"", &fontName}
    });

    if (configFound)
    {
        ffStrbufSubstrBeforeLastC(&useSysFont, '"');
        ffStrbufSubstrBeforeLastC(&fontName, '"');
    }
    else
    {
        path = "xfce4/terminal/terminalrc";
        configFound = ffParsePropFileConfigValues(path, 2, (FFpropquery[]) {
            {"FontUseSystem = ", &useSysFont},
            {"FontName = ", &fontName}
        });
    }

    if(configFound && (useSysFont.length == 0 || ffStrbufIgnCaseCompS(&useSysFont, "false") == 0))
    {
        if(fontName.length == 0)
            ffStrbufAppendF(&terminalFont->error, "Couldn't find FontName in %s", path);
        else
            ffFontInitPango(&terminalFont->font, fontName.chars);
    }
    else
    {
        const char* systemFontName = ffSettingsGetXFConf("xsettings", "/Gtk/MonospaceFontName", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(systemFontName))
            ffFontInitPango(&terminalFont->font, systemFontName);
        else
            ffStrbufAppendS(&terminalFont->error, "Couldn't find xsettings::/Gtk/MonospaceFontName in XFConf");
    }
}

static void detectDeepinTerminal(FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    FF_STRBUF_AUTO_DESTROY profile = ffStrbufCreateA(64);
    ffSearchUserConfigFile(&instance.state.platform.configDirs, "deepin/deepin-terminal/config.conf", &profile);
    FILE* file = fopen(profile.chars, "r");

    if(file)
    {
        char* line = NULL;
        size_t len = 0;

        for(int count = 0; getline(&line, &len, file) != -1 && count < 2;)
        {
            if(ffStrEquals(line, "[basic.interface.font]\n"))
            {
                if(getline(&line, &len, file) != -1)
                    ffParsePropLine(line, "value=", &fontName);
                ++count;
            }
            else if(ffStrEquals(line, "[basic.interface.font_size]\n"))
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

static void detectFootTerminal(FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY font = ffStrbufCreate();

    if (!ffParsePropFileConfig("foot/foot.ini", "font=", &font) || !ffStrSet(font.chars))
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

static void detectQTerminal(FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    ffParsePropFileConfigValues("qterminal.org/qterminal.ini", 2, (FFpropquery[]) {
        {"fontFamily=", &fontName},
        {"fontSize=", &fontSize},
    });

    if (fontName.length == 0)
        ffStrbufAppendS(&fontName, "monospace");
    if (fontSize.length == 0)
        ffStrbufAppendS(&fontSize, "12");
    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);
}

static void detectXterm(FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    ffParsePropFileHomeValues(".Xresources", 2, (FFpropquery[]) {
        {"xterm*faceName:", &fontName},
        {"xterm*faceSize:", &fontSize},
    });

    if (fontName.length == 0)
        ffStrbufAppendS(&fontName, "fixed");
    if (fontSize.length == 0)
        ffStrbufAppendS(&fontSize, "8.0");
    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);
}

static bool extractStTermFont(const char* str, FF_MAYBE_UNUSED uint32_t len, void* userdata)
{
    if (!ffStrContains(str, "size=")) return true;
    ffStrbufSetNS((FFstrbuf*) userdata, len, str);
    return false;
}

static void detectSt(FFTerminalFontResult* terminalFont, const FFTerminalResult* terminal)
{
    FF_STRBUF_AUTO_DESTROY size = ffStrbufCreateF("/proc/%u/cmdline", terminal->pid);
    FF_STRBUF_AUTO_DESTROY font = ffStrbufCreate();
    if (!ffAppendFileBuffer(size.chars, &font))
    {
        ffStrbufAppendF(&terminalFont->error, "Failed to open %s", size.chars);
        return;
    }

    const char* p = memmem(font.chars, font.length, "\0-f", sizeof("\0-f")); // find parameter of `-f`
    if (p)
    {
        // st was executed with `-f` parameter
        ffStrbufSubstrAfter(&font, (uint32_t) (p + (sizeof("\0-f") - 1) - font.chars));
        ffStrbufRecalculateLength(&font);
    }
    else
    {
        ffStrbufClear(&font);

        const char* error = ffBinaryExtractStrings(terminal->exePath.chars, extractStTermFont, &font, (uint32_t) strlen("size=0"));
        if (error)
        {
            ffStrbufAppendS(&terminalFont->error, error);
            return;
        }
        if (font.length == 0)
        {
            ffStrbufAppendS(&terminalFont->error, "No font config found in st binary");
            return;
        }
    }

    // JetBrainsMono Nerd Font Mono:pixelsize=12:antialias=true:autohint=true

    uint32_t index = ffStrbufFirstIndexC(&font, ':');
    if (index != font.length)
    {
        uint32_t sIndex = ffStrbufNextIndexS(&font, index + 1, "size=");
        if (sIndex != font.length)
        {
            sIndex += (uint32_t) strlen("size=");
            uint32_t sIndexEnd = ffStrbufNextIndexC(&font, sIndex, ':');
            ffStrbufSetNS(&size, sIndexEnd - sIndex, font.chars + sIndex);
        }
        ffStrbufSubstrBefore(&font, index);
    }
    else
        ffStrbufClear(&size);
    ffFontInitValues(&terminalFont->font, font.chars, size.chars);
}

static void detectWarp(FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateA(64);

    FF_LIST_FOR_EACH(FFstrbuf, dirPrefix, instance.state.platform.configDirs)
    {
        //We need to copy the dir each time, because it used by multiple threads, so we can't directly write to it.
        ffStrbufSet(&baseDir, dirPrefix);
        ffStrbufAppendS(&baseDir, "warp-terminal/user_preferences.json");

        yyjson_doc* doc = yyjson_read_file(baseDir.chars, YYJSON_READ_INSITU | YYJSON_READ_ALLOW_TRAILING_COMMAS | YYJSON_READ_ALLOW_COMMENTS, NULL, NULL);
        if (!doc) continue;

        yyjson_val* prefs = yyjson_obj_get(yyjson_doc_get_root(doc), "prefs");
        if (yyjson_is_obj(prefs))
        {
            const char* fontName = yyjson_get_str(yyjson_obj_get(prefs, "FontName"));
            if (!fontName) fontName = "Hack";
            const char* fontSize = yyjson_get_str(yyjson_obj_get(prefs, "FontSize"));
            if (!fontSize) fontSize = "13";

            ffFontInitValues(&terminalFont->font, fontName, fontSize);
        }
        yyjson_doc_free(doc);
        return;
    }
}

static void detectTerminator(FFTerminalFontResult* result)
{
    FF_STRBUF_AUTO_DESTROY useSystemFont = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();

    if(!ffParsePropFileConfigValues("terminator/config", 2, (FFpropquery[]) {
        {"use_system_font =", &useSystemFont},
        {"font =", &fontName},
    }) || ffStrbufIgnCaseEqualS(&useSystemFont, "True"))
    {
        FF_AUTO_FREE const char* fontName = getSystemMonospaceFont();
        if(ffStrSet(fontName))
            ffFontInitPango(&result->font, fontName);
        else
            ffStrbufAppendS(&result->error, "Couldn't get system monospace font name from GSettings / DConf");
        return;
    }

    if(fontName.length == 0)
        ffFontInitValues(&result->font, "Mono", "10");
    else
        ffFontInitPango(&result->font, fontName.chars);
}

static void detectWestonTerminal(FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY font = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY size = ffStrbufCreate();
    ffParsePropFileConfigValues("weston.ini", 2, (FFpropquery[]) {
        {"font=", &font},
        {"font-size=", &size},
    });
    if (!font.length) ffStrbufSetStatic(&font, "DejaVu Sans Mono");
    if (!size.length) ffStrbufSetStatic(&size, "14");
    ffFontInitValues(&terminalFont->font, font.chars, size.chars);
}

void ffDetectTerminalFontPlatform(const FFTerminalResult* terminal, FFTerminalFontResult* terminalFont)
{
    if(ffStrbufIgnCaseEqualS(&terminal->processName, "konsole"))
        detectKonsole(terminalFont, "konsolerc");
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "yakuake"))
        detectKonsole(terminalFont, "yakuakerc");
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "xfce4-terminal"))
        detectXFCETerminal(terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "lxterminal"))
        detectFromConfigFile("lxterminal/lxterminal.conf", "fontname =", terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "tilix"))
        detectFromGSettings("/com/gexperts/Tilix/profiles/", "com.gexperts.Tilix.ProfilesList", "com.gexperts.Tilix.Profile", "default", terminalFont);
    else if(ffStrbufStartsWithIgnCaseS(&terminal->processName, "gnome-terminal"))
        detectFromGSettings("/org/gnome/terminal/legacy/profiles:/:", "org.gnome.Terminal.ProfilesList", "org.gnome.Terminal.Legacy.Profile", "default", terminalFont);
    else if(ffStrbufStartsWithIgnCaseS(&terminal->processName, "ptyxis-agent"))
        detectPtyxis(terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "kgx"))
        detectKgx(terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "mate-terminal"))
        detectFromGSettings("/org/mate/terminal/profiles/", "org.mate.terminal.global", "org.mate.terminal.profile", "default-profile", terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "deepin-terminal"))
        detectDeepinTerminal(terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "foot"))
        detectFootTerminal(terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "qterminal"))
        detectQTerminal(terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "xterm"))
        detectXterm(terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "st"))
        detectSt(terminalFont, terminal);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "warp"))
        detectWarp(terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "weston-terminal"))
        detectWestonTerminal(terminalFont);
    else if(ffStrbufStartsWithIgnCaseS(&terminal->processName, "terminator"))
        detectTerminator(terminalFont);
}
