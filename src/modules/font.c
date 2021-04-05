#include "fastfetch.h"

#include <unistd.h>

void ffPrintFont(FFinstance* instance)
{
    FF_STRBUF_CREATE(plasmaFile);
    ffStrbufAppendS(&plasmaFile, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&plasmaFile, "/.config/kdeglobals");

    //When using Noto Sans 10, the default font in KDE Plasma, the font entry is deleted from the config file instead of set.
    //So the pure existence of the file sets this font value if not set other in the file itself.
    bool plasmaFileExists = access(plasmaFile.chars, F_OK) == 0;

    char plasma[256];
    plasma[0] = '\0';
    if(plasmaFileExists)
        ffParsePropFile(plasmaFile.chars, "font=%[^\n]", plasma);

    ffStrbufDestroy(&plasmaFile);

    FFstrbuf* gtk2;
    ffCalculateGTK2(instance, NULL, NULL, &gtk2);

    FFstrbuf* gtk3;
    ffCalculateGTK3(instance, NULL, NULL, &gtk3);

    FFstrbuf* gtk4;
    ffCalculateGTK4(instance, NULL, NULL, &gtk4);

    if(plasma[0] == '\0' && !plasmaFileExists && gtk2->length == 0 && gtk3->length == 0 && gtk4->length == 0)
    {
        ffPrintError(instance, &instance->config.fontKey, "Font", "No fonts found");
        return;
    }

    FF_STRBUF_CREATE(plasmaName);
    FF_STRBUF_CREATE(plasmaPretty);
    double plasmaSize;

    if(plasma[0] == '\0' && plasmaFileExists)
    {
        strcpy(plasma, "Noto Sans, 10");
        ffStrbufSetS(&plasmaName, "Noto Sans");
        plasmaSize = 10;
        ffStrbufSetS(&plasmaPretty, "Noto Sans (10pt)");
    }
    else
    {
        ffParseFont(plasma, &plasmaName, &plasmaSize);
        ffFontPretty(&plasmaPretty, &plasmaName, plasmaSize);
    }

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

        if(plasma[0] != '\0')
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
}
