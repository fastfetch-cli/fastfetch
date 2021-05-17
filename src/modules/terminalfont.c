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

static void printTilixTerminal(FFinstance* instance)
{    // Note - Subject to change when DConf/GSettings fix their implementations of defaults/relocatable schemas respectively
    const char* fontName = NULL;

    const char* defaultProfile = ffSettingsGetGsettings(instance, "com.gexperts.Tilix.ProfilesList", NULL, "default", FF_VARIANT_TYPE_STRING).strValue;

    if(!defaultProfile)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't find \"Default\" profile in Tilix settings");
        return;
    }

    FFstrbuf key;
    ffStrbufInitAS(&key, 64, "/com/gexperts/Tilix/profiles/");
    ffStrbufAppendS(&key, defaultProfile);
    uint32_t keyLen = key.length;
    ffStrbufAppendS(&key, "/use-system-font");

    FFvariant res = ffSettingsGetDConf(instance,key.chars, FF_VARIANT_TYPE_BOOL);

    if(res.boolValueSet && !res.boolValue) // custom font
    {
        ffStrbufSubstrBefore(&key, keyLen);
        ffStrbufAppendS(&key, "/font");
        fontName = ffSettingsGetDConf(instance, key.chars, FF_VARIANT_TYPE_STRING).strValue;
    }
    else if(!res.boolValueSet || res.boolValue) // system font
        fontName = ffSettingsGetDConf(instance, "/org/gnome/desktop/interface/monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;

    if(fontName == NULL)
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't find \"Terminal font\" in Tilix settings");
    else
        printTerminalFont(instance, fontName);

    ffStrbufDestroy(&key);
}

static void printGnomeTerminal(FFinstance* instance)
{
    const char* defaultProfile = ffSettingsGetGsettings(instance, "org.gnome.Terminal.ProfilesList", NULL, "default", FF_VARIANT_TYPE_STRING).strValue;

    if(!defaultProfile)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't get \"Default\" profile from gsettings");
        return;
    }

    const char* fontName = NULL;
    FFstrbuf path;
    ffStrbufInitAS(&path, 128, "/org/gnome/terminal/legacy/profiles:/:");
    ffStrbufAppendS(&path, defaultProfile);
    ffStrbufAppendC(&path, '/');

    FFvariant res = ffSettingsGetGsettings(instance, "org.gnome.Terminal.Legacy.Profile", path.chars, "use-system-font", FF_VARIANT_TYPE_BOOL);

    if(!res.boolValue) // custom font
        fontName = ffSettingsGetGsettings(instance, "org.gnome.Terminal.Legacy.Profile", path.chars, "font", FF_VARIANT_TYPE_STRING).strValue;
    else // system font
        fontName = ffSettingsGetGsettings(instance, "org.gnome.desktop.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;

    if(fontName == NULL)
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't get Gnome terminal font from gsettings");
    else
        printTerminalFont(instance, fontName);

    ffStrbufDestroy(&path);
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
        printTilixTerminal(instance);
    else if(ffStrbufIgnCaseCompS(&result->exeName, "gnome-terminal-server") == 0)
        printGnomeTerminal(instance);
    else if(ffStrbufStartsWithIgnCaseS(&result->exeName, "/dev/tty"))
        printTTY(instance);
    else
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Unknown terminal: %s", result->exeName.chars);
}
