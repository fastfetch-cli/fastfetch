#include "fastfetch.h"

static void printWMTheme(FFinstance* instance, const char* theme)
{
    if(instance->config.wmThemeFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, &instance->config.wmThemeKey, "WM Theme");
        puts(theme);
    }
    else
    {
        ffPrintFormatString(instance, &instance->config.wmThemeKey, "WM Theme", &instance->config.wmThemeFormat, 1,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, theme}
        );
    }
}

static void printKWin(FFinstance* instance)
{
    char theme[256];
    theme[0] = '\0';
    ffParsePropFileHome(instance, ".config/kwinrc", "theme=%s", theme);

    if(theme[0] == '\0')
    {
        ffPrintError(instance, &instance->config.wmThemeKey, "WM Theme", "Couldn't find \"theme=\" in \".config/kwinrc\"");
        return;
    }

    printWMTheme(instance, theme);
}

void ffPrintWMTheme(FFinstance* instance)
{
    FFstrbuf* wmName;
    ffCalculateWM(instance, &wmName, NULL, NULL);

    if(wmName->length == 0)
    {
        ffPrintError(instance, &instance->config.wmThemeKey, "WM Theme", "WM Theme needs sucessfull WM detection");
        return;
    }

    if(ffStrbufIgnCaseCompS(wmName, "KWin") == 0)
        printKWin(instance);
    else
        ffPrintError(instance, &instance->config.wmThemeKey, "WM Theme", "Unknown WM: %s", wmName->chars);
}
