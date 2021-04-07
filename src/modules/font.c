#include "fastfetch.h"

#include <unistd.h>

void ffPrintFont(FFinstance* instance)
{
    FFstrbuf* plasma;
    ffCalculatePlasma(instance, NULL, NULL, &plasma);

    FFstrbuf* gtk2;
    ffCalculateGTK2(instance, NULL, NULL, &gtk2);

    FFstrbuf* gtk3;
    ffCalculateGTK3(instance, NULL, NULL, &gtk3);

    FFstrbuf* gtk4;
    ffCalculateGTK4(instance, NULL, NULL, &gtk4);

    if(plasma->length == 0 && gtk2->length == 0 && gtk3->length == 0 && gtk4->length == 0)
    {
        ffPrintError(instance, &instance->config.fontKey, "Font", "No fonts found");
        return;
    }

    FF_STRBUF_CREATE(plasmaName);
    double plasmaSize;
    ffParseFont(plasma->chars, &plasmaName, &plasmaSize);
    FF_STRBUF_CREATE(plasmaPretty);
    ffFontPretty(&plasmaPretty, &plasmaName, plasmaSize);

    FF_STRBUF_CREATE(gtk2Name);
    double gtk2Size;
    ffParseFont(gtk2->chars, &gtk2Name, &gtk2Size);
    FF_STRBUF_CREATE(gtk2Pretty);
    ffFontPretty(&gtk2Pretty, &gtk2Name, gtk2Size);

    FF_STRBUF_CREATE(gtk3Name);
    double gtk3Size;
    ffParseFont(gtk3->chars, &gtk3Name, &gtk3Size);
    FF_STRBUF_CREATE(gtk3Pretty);
    ffFontPretty(&gtk3Pretty, &gtk3Name, gtk3Size);

    FF_STRBUF_CREATE(gtk4Name);
    double gtk4Size;
    ffParseFont(gtk4->chars, &gtk4Name, &gtk4Size);
    FF_STRBUF_CREATE(gtk4Pretty);
    ffFontPretty(&gtk4Pretty, &gtk4Name, gtk4Size);

    FF_STRBUF_CREATE(gtkPretty);
    ffFormatGtkPretty(&gtkPretty, &gtk2Pretty, &gtk3Pretty, &gtk4Pretty);

    if(instance->config.fontFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, &instance->config.fontKey, "Font");

        if(plasma->length > 0)
        {
            ffStrbufWriteTo(&plasmaPretty, stdout);
            fputs(" [Plasma]", stdout);

            if(gtkPretty.length > 0)
                fputs(", ", stdout);
        }
        ffStrbufPutTo(&gtkPretty, stdout);
    }
    else
    {
        ffPrintFormatString(instance, &instance->config.fontKey, "Font", &instance->config.fontFormat, 17,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, plasma},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &plasmaName},
            (FFformatarg){FF_FORMAT_ARG_TYPE_DOUBLE, &plasmaSize},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &plasmaPretty},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, gtk2},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &gtk2Name},
            (FFformatarg){FF_FORMAT_ARG_TYPE_DOUBLE, &gtk2Size},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &gtk2Pretty},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, gtk3},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &gtk3Name},
            (FFformatarg){FF_FORMAT_ARG_TYPE_DOUBLE, &gtk3Size},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &gtk3Pretty},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, gtk4},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &gtk4Name},
            (FFformatarg){FF_FORMAT_ARG_TYPE_DOUBLE, &gtk4Size},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &gtk4Pretty},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &gtkPretty}
        );
    }

    ffStrbufDestroy(&plasmaName);
    ffStrbufDestroy(&plasmaPretty);

    ffStrbufDestroy(&gtk2Name);
    ffStrbufDestroy(&gtk2Pretty);

    ffStrbufDestroy(&gtk3Name);
    ffStrbufDestroy(&gtk3Pretty);

    ffStrbufDestroy(&gtk3Name);
    ffStrbufDestroy(&gtk3Pretty);

    ffStrbufDestroy(&gtkPretty);
}
