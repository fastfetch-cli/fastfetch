#include "fastfetch.h"

static void printTerminalFont(FFinstance* instance, char* font)
{
    ffPrintLogoAndKey(instance, "Terminal font");

    if(ffStrbufIsEmpty(&instance->config.termFontFormat))
    {
        puts(font);
    }
    else
    {
        FF_STRBUF_CREATE(termfont);

        ffParseFormatString(&termfont, &instance->config.termFontFormat, 1,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, font}
        );

        ffStrbufPutTo(&termfont, stdout);
        ffStrbufDestroy(&termfont);
    }
}

static void printKonsole(FFinstance* instance)
{
    char profile[128];
    ffParsePropFileHome(instance, ".config/konsolerc", "DefaultProfile=%[^\n]", profile);
    if(profile[0] == '\0')
    {
        ffPrintError(instance, "Terminal Font", "Couldn't find \"DefaultProfile=%[^\n]\" in \".config/konsolerc\"");
        return;
    }

    char profilePath[256];
    sprintf(profilePath, ".local/share/konsole/%s", profile);

    char font[128];
    ffParsePropFileHome(instance, profilePath, "Font=%[^\n]", font);
    if(font[0] == '\0')
    {
        ffPrintError(instance, "Terminal Font", "Couldn't find \"Font=%%[^\\n]\" in \"%s\"", profilePath);
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
    ffPopulateTerminal(instance);
    
    if(instance->state.terminal.error != NULL)
    {
        ffPrintError(instance, "Terminal Font", "Terminal Font needs successfull terminal detection");
        return;
    }

    if(ffStrbufIgnCaseCompS(&instance->state.terminal.value, "konsole") == 0)
        printKonsole(instance);
    else if(ffStrbufIgnCaseCompS(&instance->state.terminal.value, "TTY") == 0)
        printTTY(instance);
    else
        ffPrintError(instance, "Terminal Font", "Unknown terminal: %s", instance->state.terminal.value);
}