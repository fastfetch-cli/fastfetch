#include "fastfetch.h"

void ffPrintFont(FFinstance* instance)
{
    char plasma[256];
    ffParsePropFileHome(instance, ".config/kdeglobals", "font=%[^\n]", plasma);

    char plasmaPretty[256];
    if(plasma[0] == '\0')
        strcpy(plasmaPretty, "Noto Sans (10pt)");
    else
        ffParseFont(plasma, plasmaPretty);

    char gtk2[256];
    ffParsePropFileHome(instance, ".gtkrc-2.0", "gtk-font-name=\"%[^\"]+", gtk2);

    char gtk2Pretty[256];
    if(gtk2[0] == '\0')
        gtk2Pretty[0] = '\0';
    else
        ffParseFont(gtk2, gtk2Pretty);

    char gtk3[256];
    ffParsePropFileHome(instance, ".config/gtk-3.0/settings.ini", "gtk-font-name=%[^\n]", gtk3);

    char gtk3Pretty[256];
    if(gtk3[0] == '\0')
        gtk3Pretty[0] = '\0';
    else
        ffParseFont(gtk3, gtk3Pretty);

    char gtk4[256];
    ffParsePropFileHome(instance, ".config/gtk-4.0/settings.ini", "gtk-font-name=%[^\n]", gtk4);

    char gtk4Pretty[256];
    if(gtk4Pretty[0] == '\0')
        gtk4Pretty[0] = '\0';
    else
        ffParseFont(gtk4, gtk4Pretty);

    FF_STRBUF_CREATE(gtkPretty);
    ffFormatGtkPretty(&gtkPretty, gtk2Pretty, gtk3Pretty, gtk4Pretty);

    if(instance->config.fontFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, &instance->config.fontKey, "Font");
        printf("%s [Plasma], ", plasmaPretty);
        ffStrbufPutTo(&gtkPretty, stdout);
    }
    else
    {
        ffPrintFormatString(instance, &instance->config.fontKey, "Font", &instance->config.fontFormat, 5,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, plasmaPretty},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, gtk2Pretty},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, gtk3Pretty},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, gtk4Pretty},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &gtkPretty}
        );
    }

    ffStrbufDestroy(&gtkPretty);
}
