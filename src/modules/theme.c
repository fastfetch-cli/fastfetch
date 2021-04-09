#include "fastfetch.h"

void ffPrintTheme(FFinstance* instance)
{
    FFstrbuf* plasma;
    ffCalculatePlasma(instance, &plasma, NULL, NULL);

    FFstrbuf* gtk2;
    ffCalculateGTK2(instance, &gtk2, NULL, NULL);

    FFstrbuf* gtk3;
    ffCalculateGTK3(instance, &gtk3, NULL, NULL);

    FFstrbuf* gtk4;
    ffCalculateGTK4(instance, &gtk4, NULL, NULL);

    if(plasma->length == 0 && gtk2->length == 0 && gtk3->length == 0 && gtk4->length == 0)
    {
        ffPrintError(instance, &instance->config.themeKey, "Theme", "No themes found");
        return;
    }

    FF_STRBUF_CREATE(gtkPretty);
    ffGetGtkPretty(&gtkPretty, gtk2, gtk3, gtk4);

    if(instance->config.themeFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, &instance->config.themeKey, "Theme");

        if(plasma->length > 0)
        {
            ffStrbufWriteTo(plasma, stdout);
            fputs(" [Plasma]", stdout);

            if(gtkPretty.length > 0)
                fputs(", ", stdout);
        }

        ffStrbufPutTo(&gtkPretty, stdout);
    }
    else
    {
        ffPrintFormatString(instance, &instance->config.themeKey, "Theme", &instance->config.themeFormat, 5,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, plasma},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, gtk2},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, gtk3},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, gtk4},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &gtkPretty}
        );
    }
}
