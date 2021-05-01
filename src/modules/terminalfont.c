#include "fastfetch.h"

#include <string.h>

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
    ffParsePropFileHome(instance, ".config/konsolerc", "DefaultProfile=%[^\n]", profile);
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
    ffParsePropFileHome(instance, ".config/xfce4/terminal/terminalrc", "FontName=%[^\n]", font);
    if(font[0] == '\0')
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Couldn't find \"FontName=%%[^\\n]\" in \".config/xfce4/terminal/terminalrc\"");
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
    else if(ffStrbufStartsWithIgnCaseS(&result->exeName, "/dev/tty"))
        printTTY(instance);
    else
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.termFontKey, &instance->config.termFontFormat, FF_TERMFONT_NUM_FORMAT_ARGS, "Unknown terminal: %s", result->exeName.chars);
}
