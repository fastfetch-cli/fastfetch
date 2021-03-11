#include "fastfetch.h"

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
        char error[289];
        sprintf(error, "Couldn't find \"Font=%%[^\\n]\" in \"%s\"", profilePath);
        ffPrintError(instance, "Terminal Font", error);
    }

    char fontPretty[128];
    ffParseFont(font, fontPretty);

    ffPrintLogoAndKey(instance, "Terminal Font");
    puts(fontPretty);
}

static void printTTY(FFinstance* instance)
{
    char font[128];
    ffParsePropFile("/etc/vconsole.conf", "Font=%[^\n]", font);
    if(font[0] == '\0')
        strcpy(font, "hardware-supplied VGA font");
    
    ffPrintLogoAndKey(instance, "Terminal Font");
    puts(font);
}

void ffPrintTerminalFont(FFinstance* instance)
{
    ffPopulateTerminal(instance);
    
    if(instance->state.terminal.error != NULL)
    {
        ffPrintError(instance, "Terminal Font", "Terminal Font needs successfull terminal detection");
    }
    else if(instance->state.terminal.value == NULL)
    {
        ffPrintError(instance, "Terminal Font", "ffPopulateTerminal failed");
    }

    if(strcasecmp(instance->state.terminal.value, "konsole") == 0)
        printKonsole(instance);
    else if(strcasecmp(instance->state.terminal.value, "TTY") == 0)
        printTTY(instance);
    else
    {
        char error[256];
        sprintf(error, "Unknown terminal: %s", instance->state.terminal.value);
        ffPrintError(instance, "Terminal Font", error);
    }

}