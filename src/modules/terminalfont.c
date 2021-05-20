#include "fastfetch.h"

#define FF_TERMFONT_MODULE_NAME "Terminal Font"
#define FF_TERMFONT_NUM_FORMAT_ARGS 4

static void printTerminalFont(FFinstance* instance, const char* font)
{
    FF_STRBUF_CREATE(name);
    double size;
    ffGetFont(font, &name, &size);
    FF_STRBUF_CREATE(pretty);
    ffGetFontPretty(&pretty, &name, size);

    if(instance->config.termFontFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey);
        ffStrbufPutTo(&pretty, stdout);
    }
    else
    {
        ffPrintFormatString(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, NULL, FF_TERMFONT_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, font},
            {FF_FORMAT_ARG_TYPE_STRBUF, &name},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &size},
            {FF_FORMAT_ARG_TYPE_STRBUF, &pretty}
        });
    }

    ffStrbufDestroy(&name);
    ffStrbufDestroy(&pretty);
}

static const char* getSystemMonospaceFont(FFinstance* instance)
{
    const FFWMDEResult* wmde = ffDetectWMDE(instance);

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
    FFstrbuf font;
    ffStrbufInit(&font);
    ffParsePropFileConfig(instance, configFile, start, &font);

    if(font.length == 0)
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't find terminal font in \"$XDG_CONFIG_HOME/%s\"", configFile);
    else
        printTerminalFont(instance, font.chars);

    ffStrbufDestroy(&font);
}

static void printTerminalFontFromGSettings(FFinstance* instance, char* profilePath, char* profileList, char* profile)
{
    const char* defaultProfile = ffSettingsGetGSettings(instance, profileList, NULL, "default", FF_VARIANT_TYPE_STRING).strValue;
    if(defaultProfile == NULL)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't get \"default\" profile from gsettings");
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
            ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't get terminal font from GSettings (%s::%s::font)", profile, path.chars);
    }
    else // system font
    {
        fontName = getSystemMonospaceFont(instance);
        if(fontName == NULL)
            ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Could't get system monospace font name from GSettings / DConf");
    }

    ffStrbufDestroy(&path);

    if(fontName == NULL)
        return;

    printTerminalFont(instance, fontName);
}

static void printKonsole(FFinstance* instance)
{
    FFstrbuf profile;
    ffStrbufInit(&profile);
    ffParsePropFileConfig(instance, "konsolerc", "DefaultProfile =", &profile);

    if(profile.length == 0)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't find \"DefaultProfile=%[^\\n]\" in \".config/konsolerc\"");
        ffStrbufDestroy(&profile);
        return;
    }

    FFstrbuf profilePath;
    ffStrbufInitA(&profilePath, 64);
    ffStrbufAppendS(&profilePath, ".local/share/konsole/");
    ffStrbufAppend(&profilePath, &profile);

    FFstrbuf font;
    ffStrbufInit(&font);
    ffParsePropFileHome(instance, profilePath.chars, "Font =", &font);

    if(font.length == 0)
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't find \"Font=%%[^\\n]\" in \"%s\"", profilePath.chars);
    else
        printTerminalFont(instance, font.chars);

    ffStrbufDestroy(&font);
    ffStrbufDestroy(&profilePath);
    ffStrbufDestroy(&profile);
}

static void printXCFETerminal(FFinstance* instance)
{
    FFstrbuf useSysFont;
    ffStrbufInit(&useSysFont);

    if(!ffParsePropFileConfig(instance, "xfce4/terminal/terminalrc", "FontUseSystem =", &useSysFont))
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't open \"$XDG_CONFIG_HOME/xfce4/terminal/terminalrc\"");
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
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't find \"xsettings::/Gtk/MonospaceFontName\" in XFConf");
    else
        printTerminalFont(instance, fontName);
}

static void printTTY(FFinstance* instance)
{
    FFstrbuf font;
    ffStrbufInit(&font);

    ffParsePropFile("/etc/vconsole.conf", "Font =", &font);

    if(font.length == 0)
    {
        ffStrbufAppendS(&font, "VGA default kernel font ");
        ffProcessAppendStdOut(&font, (char* const[]){
            "showconsolefont",
            "--info",
            NULL
        });
    }

    printTerminalFont(instance, font.chars);
    ffStrbufDestroy(&font);
}

void ffPrintTerminalFont(FFinstance* instance)
{
    const FFTerminalResult* result = ffDetectTerminal(instance);

    if(result->exeName.length == 0)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Terminal font needs successfull terminal detection");
        return;
    }

    if(ffStrbufIgnCaseCompS(&result->exeName, "konsole") == 0)
        printKonsole(instance);
    else if(ffStrbufIgnCaseCompS(&result->exeName, "xfce4-terminal") == 0)
        printXCFETerminal(instance);
    else if(ffStrbufIgnCaseCompS(&result->exeName, "lxterminal") == 0)
        printTerminalFontFromConfigFile(instance, "lxterminal/lxterminal.conf", "fontname =");
    else if(ffStrbufIgnCaseCompS(&result->exeName, "tilix") == 0)
        printTerminalFontFromGSettings(instance, "/com/gexperts/Tilix/profiles/", "com.gexperts.Tilix.ProfilesList", "com.gexperts.Tilix.Profile");
    else if(ffStrbufIgnCaseCompS(&result->exeName, "gnome-terminal-server") == 0)
        printTerminalFontFromGSettings(instance, "/org/gnome/terminal/legacy/profiles:/:", "org.gnome.Terminal.ProfilesList", "org.gnome.Terminal.Legacy.Profile");
    else if(ffStrbufStartsWithIgnCaseS(&result->exeName, "/dev/tty"))
        printTTY(instance);
    else
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Unknown terminal: %s", result->exeName.chars);
}
