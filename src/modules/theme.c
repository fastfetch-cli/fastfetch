#include "fastfetch.h"

void ffPrintTheme(FFinstance* instance)
{
    char plasma[256];
    ffParsePropFileHome(instance, ".config/kdeglobals", "Name=%[^\n]", plasma);

    FFstrbuf* gtk2;
    ffCalculateGTK2(instance, &gtk2, NULL, NULL);

    FFstrbuf* gtk3;
    ffCalculateGTK3(instance, &gtk3, NULL, NULL);

    FFstrbuf* gtk4;
    ffCalculateGTK4(instance, &gtk4, NULL, NULL);

    if(plasma[0] == '\0' && gtk2->length == 0 && gtk3->length == 0 && gtk4->length == 0)
    {
        ffPrintError(instance, &instance->config.themeKey, "Theme", "No themes found");
        return;
    }

    FF_STRBUF_CREATE(gtkPretty);
    ffFormatGtkPretty(&gtkPretty, gtk2, gtk3, gtk4);

    if(instance->config.themeFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, &instance->config.themeKey, "Theme");

        if(plasma[0] != '\0')
            printf("%s [Plasma], ", plasma);

        ffStrbufPutTo(&gtkPretty, stdout);
    }
    else
    {
        ffPrintFormatString(instance, &instance->config.themeKey, "Theme", &instance->config.themeFormat, 5,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, plasma},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, gtk2},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, gtk3},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, gtk4},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &gtkPretty}
        );
    }
}
