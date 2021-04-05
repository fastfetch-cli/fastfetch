#include "fastfetch.h"

void ffPrintIcons(FFinstance* instance)
{
    char plasma[256];
    ffParsePropFileHome(instance, ".config/kdeglobals", "Theme=%[^\n]", plasma);

    FFstrbuf* gtk2;
    ffCalculateGTK2(instance, NULL, &gtk2, NULL);

    FFstrbuf* gtk3;
    ffCalculateGTK3(instance, NULL, &gtk3, NULL);

    FFstrbuf* gtk4;
    ffCalculateGTK4(instance, NULL, &gtk4, NULL);

    if(plasma[0] == '\0' && gtk2->length == 0 && gtk3->length == 0 && gtk4->length == 0)
    {
        ffPrintError(instance, &instance->config.iconsKey, "Icons", "No icons found");
        return;
    }

    FF_STRBUF_CREATE(gtkPretty);
    ffFormatGtkPretty(&gtkPretty, gtk2, gtk3, gtk4);

    if(instance->config.iconsFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, &instance->config.iconsKey, "Icons");

        if(plasma[0] == '\0')
            printf("Breeze [Plasma], ");
        else
            printf("%s [Plasma], ", plasma);

        ffStrbufPutTo(&gtkPretty, stdout);
    }
    else
    {
        ffPrintFormatString(instance, &instance->config.iconsKey, "Icons", &instance->config.iconsFormat, 5,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, plasma},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, gtk2},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, gtk3},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, gtk4},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &gtkPretty}
        );
    }

    ffStrbufDestroy(&gtkPretty);
}
