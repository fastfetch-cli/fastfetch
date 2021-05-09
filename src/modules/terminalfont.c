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

static void printKonsole(FFinstance* instance)
{
    char profile[128];
    ffParsePropFileConfig(instance, "konsolerc", "DefaultProfile=%[^\n]", profile);
    if(profile[0] == '\0')
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't find \"DefaultProfile=%[^\n]\" in \".config/konsolerc\"");
        return;
    }

    char profilePath[256];
    sprintf(profilePath, ".local/share/konsole/%s", profile);

    char font[128];
    ffParsePropFileHome(instance, profilePath, "Font=%[^\n]", font);
    if(font[0] == '\0')
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't find \"Font=%%[^\\n]\" in \"%s\"", profilePath);
        return;
    }

    printTerminalFont(instance, font);
}

static void printXFCE4Terminal(FFinstance* instance)
{
    char font[128];
    ffParsePropFileConfig(instance, "xfce4/terminal/terminalrc", "FontName=%[^\n]", font);
    if(font[0] == '\0')
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't find \"FontName=%%[^\\n]\" in \".config/xfce4/terminal/terminalrc\"");
        return;
    }

    printTerminalFont(instance, font);
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

    if(!fontName)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't find \"Terminal font\" in Tilix settings");
        ffStrbufDestroy(&key);
        return;
    }

    printTerminalFont(instance, fontName);
    ffStrbufDestroy(&key);
}

static void printLxTerminal(FFinstance* instance)
{
    char font[128];
    ffParsePropFileConfig(instance, "lxterminal/lxterminal.conf", "fontname=%[^\n]", font);
    if(font[0] == '\0')
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't find \"fontname=%%[^\\n]\" in \".config/lxterminal/lxterminal.conf\"");
        return;
    }

    printTerminalFont(instance, font);
}

static void printTTY(FFinstance* instance)
{
    FFstrbuf font;
    ffStrbufInit(&font);

    ffParsePropFile("/etc/vconsole.conf", "Font=%[^\n]", font.chars);
    ffStrbufRecalculateLength(&font);

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
        printXFCE4Terminal(instance);
    else if(ffStrbufIgnCaseCompS(&result->exeName, "tilix") == 0)
        printTilixTerminal(instance);
    else if(ffStrbufIgnCaseCompS(&result->exeName, "lxterminal") == 0)
        printLxTerminal(instance);
    else if(ffStrbufStartsWithIgnCaseS(&result->exeName, "/dev/tty"))
        printTTY(instance);
    else
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Unknown terminal: %s", result->exeName.chars);
}
