#include "fastfetch.h"

static void printTerminalFont(FFinstance* instance, char* font)
{
    if(instance->config.termFontFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, &instance->config.termFontKey, "Terminal font");
        puts(font);
    }
    else
    {
        ffPrintFormatString(instance, &instance->config.termFontKey, "Terminal font", &instance->config.termFontFormat, 1,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, font}
        );
    }
}

static void printKonsole(FFinstance* instance)
{
    char profile[128];
    ffParsePropFileHome(instance, ".config/konsolerc", "DefaultProfile=%[^\n]", profile);
    if(profile[0] == '\0')
    {
        ffPrintError(instance, &instance->config.termFontKey, "Terminal Font", "Couldn't find \"DefaultProfile=%[^\n]\" in \".config/konsolerc\"");
        return;
    }

    char profilePath[256];
    sprintf(profilePath, ".local/share/konsole/%s", profile);

    char font[128];
    ffParsePropFileHome(instance, profilePath, "Font=%[^\n]", font);
    if(font[0] == '\0')
    {
        ffPrintError(instance, &instance->config.termFontKey, "Terminal Font", "Couldn't find \"Font=%%[^\\n]\" in \"%s\"", profilePath);
        return;
    }

    char fontPretty[128];
    ffParseFont(font, fontPretty);

    printTerminalFont(instance, fontPretty);
}

static void printTTY(FFinstance* instance)
{
    char font[128];
    ffParsePropFile("/etc/vconsole.conf", "Font=%[^\n]", font);
    if(font[0] == '\0')
        strcpy(font, "hardware-supplied VGA font");

    printTerminalFont(instance, font);
}

void ffPrintTerminalFont(FFinstance* instance)
{
    FFstrbuf* terminalExeName;
    ffCalculateTerminal(instance, &terminalExeName, NULL, NULL);

    if(terminalExeName->length == 0)
    {
        ffPrintError(instance, &instance->config.termFontKey, "Terminal Font", "Terminal font needs successfull terminal detection");
        return;
    }

    if(ffStrbufIgnCaseCompS(terminalExeName, "konsole") == 0)
        printKonsole(instance);
    else if(ffStrbufIgnCaseCompS(terminalExeName, "login") == 0)
        printTTY(instance);
    else
        ffPrintError(instance, &instance->config.termFontKey, "Terminal Font", "Unknown terminal: %s", terminalExeName->chars);
}
