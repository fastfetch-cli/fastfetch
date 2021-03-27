#include "fastfetch.h"

void ffPrintIcons(FFinstance* instance)
{
    char plasma[256];
    ffParsePropFileHome(instance, ".config/kdeglobals", "Theme=%[^\n]", plasma);
    
    char gtk2[256];
    ffParsePropFileHome(instance, ".gtkrc-2.0", "gtk-icon-theme-name=\"%[^\"]+", gtk2);
    if(gtk2[0] == '\0')
        strcpy(gtk2, "Adwaita");

    char gtk3[256];
    ffParsePropFileHome(instance, ".config/gtk-3.0/settings.ini", "gtk-icon-theme-name=%[^\n]", gtk3);
    if(gtk3[0] == '\0')
        strcpy(gtk3, "Adwaita");

    char gtk4[256];
    ffParsePropFileHome(instance, ".config/gtk-4.0/settings.ini", "gtk-icon-theme-name=%[^\n]", gtk4);
    if(gtk4[0] == '\0')
        strcpy(gtk4, "Adwaita");

    FF_STRBUF_CREATE(gtkPretty);
    ffFormatGtkPretty(&gtkPretty, gtk2, gtk3, gtk4);

    if(instance->config.iconsFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, "Icons");
        
        if(plasma[0] == '\0')
            printf("Breeze [Plasma], ");
        else
            printf("%s [Plasma], ", plasma);

        ffStrbufPutTo(&gtkPretty, stdout);
    }
    else
    {
        ffPrintFormatString(instance, "Icons", &instance->config.iconsFormat, 5,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, plasma},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, gtk2},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, gtk3},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, gtk4},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &gtkPretty}
        );
    }

    ffStrbufDestroy(&gtkPretty);
}