#include "fastfetch.h"

#define FF_TERMFONT_MODULE_NAME "Terminal Font"
#define FF_TERMFONT_NUM_FORMAT_ARGS 5

static void printTerminalFont(FFinstance* instance, const char* raw, FFfont* font)
{
    if(font->pretty.length == 0)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Terminal font is an empty value");
        return;
    }

    if(instance->config.terminalFont.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont.key);
        ffStrbufPutTo(&font->pretty, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, FF_TERMFONT_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, raw},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->size},
            {FF_FORMAT_ARG_TYPE_LIST,   &font->styles},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->pretty}
        });
    }
}

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
        printTerminalFont(instance, fontName.chars, &font);
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
    printTerminalFont(instance, fontName, &font);
    ffFontDestroy(&font);
}

static void printKonsole(FFinstance* instance)
{
    FFstrbuf profile;
    ffStrbufInit(&profile);
    ffParsePropFileConfig(instance, "konsolerc", "DefaultProfile =", &profile);

    if(profile.length == 0)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Couldn't find \"DefaultProfile=%[^\\n]\" in \".config/konsolerc\"");
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
        printTerminalFont(instance, fontName.chars, &font);
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
        printTerminalFont(instance, fontName, &font);
        ffFontDestroy(&font);
    }
}

static void printAlacritty(FFinstance* instance) {
    FFstrbuf fontName;
    FFstrbuf fontSize;
    ffStrbufInit(&fontName);
    ffStrbufInit(&fontSize);

    FFpropquery fontQuery[] = {
        {"family:", &fontName},
        {"size:", &fontSize},
    };

    // alacritty parses config files in this order
    ffParsePropFileConfigValues(instance, "alacritty/alacritty.yml", 2, fontQuery);
    if(fontName.length == 0 || fontSize.length == 0)
        ffParsePropFileConfigValues(instance, "alacritty.yml", 2, fontQuery);
    if(fontName.length == 0 || fontSize.length == 0)
        ffParsePropFileConfigValues(instance, ".alacritty.yml", 2, fontQuery);

    //by default alacritty uses its own font called alacritty
    if(fontName.length == 0)
        ffStrbufAppendS(&fontName, "alacritty");

    // the default font size is 11
    if(fontSize.length == 0)
        ffStrbufAppendS(&fontSize, "11");

    FFfont font;
    ffFontInitCopy(&font, fontName.chars);

    if(fontSize.length > 0)
        ffStrbufSet(&font.size, &fontSize);

    printTerminalFont(instance, fontName.chars, &font);
    ffFontDestroy(&font);

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&fontSize);
}

static void printTTY(FFinstance* instance)
{
    FFstrbuf fontName;
    ffStrbufInit(&fontName);

    ffParsePropFile(FASTFETCH_TARGET_DIR_ROOT"/etc/vconsole.conf", "Font =", &fontName);

    if(fontName.length == 0)
    {
        ffStrbufAppendS(&fontName, "VGA default kernel font ");
        ffProcessAppendStdOut(&fontName, (char* const[]){
            "showconsolefont",
            "--info",
            NULL
        });
    }

    ffStrbufTrimRight(&fontName, ' ');

    FFfont font;
    ffFontInitCopy(&font, fontName.chars);
    printTerminalFont(instance, fontName.chars, &font);
    ffFontDestroy(&font);
    ffStrbufDestroy(&fontName);
}

void ffPrintTerminalFont(FFinstance* instance)
{
    const FFTerminalShellResult* result = ffDetectTerminalShell(instance);

    if(result->terminalProcessName.length == 0)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Terminal font needs successfull terminal detection");
        return;
    }

    if(ffStrbufIgnCaseCompS(&result->terminalProcessName, "konsole") == 0)
        printKonsole(instance);
    else if(ffStrbufIgnCaseCompS(&result->terminalProcessName, "xfce4-terminal") == 0)
        printXCFETerminal(instance);
    else if(ffStrbufIgnCaseCompS(&result->terminalProcessName, "lxterminal") == 0)
        printTerminalFontFromConfigFile(instance, "lxterminal/lxterminal.conf", "fontname =");
    else if(ffStrbufIgnCaseCompS(&result->terminalProcessName, "tilix") == 0)
        printTerminalFontFromGSettings(instance, "/com/gexperts/Tilix/profiles/", "com.gexperts.Tilix.ProfilesList", "com.gexperts.Tilix.Profile");
    else if(ffStrbufIgnCaseCompS(&result->terminalProcessName, "gnome-terminal-") == 0)
        printTerminalFontFromGSettings(instance, "/org/gnome/terminal/legacy/profiles:/:", "org.gnome.Terminal.ProfilesList", "org.gnome.Terminal.Legacy.Profile");
    else if(ffStrbufStartsWithIgnCaseS(&result->terminalExe, "/dev/tty"))
        printTTY(instance);
    else if(ffStrbufIgnCaseCompS(&result->terminalProcessName, "alacritty") == 0)
        printAlacritty(instance);
    else
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Unknown terminal: %s", result->terminalProcessName.chars);
}
