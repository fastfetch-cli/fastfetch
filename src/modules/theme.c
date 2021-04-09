#include "fastfetch.h"

void ffPrintTheme(FFinstance* instance)
{
    FFstrbuf* plasmaTheme;
    FFstrbuf* plasmaColor;
    ffCalculatePlasma(instance, &plasmaTheme, &plasmaColor, NULL, NULL);

    FFstrbuf* gtk2;
    ffCalculateGTK2(instance, &gtk2, NULL, NULL);

    FFstrbuf* gtk3;
    ffCalculateGTK3(instance, &gtk3, NULL, NULL);

    FFstrbuf* gtk4;
    ffCalculateGTK4(instance, &gtk4, NULL, NULL);

    if(plasmaTheme->length == 0 && plasmaColor->length == 0 && gtk2->length == 0 && gtk3->length == 0 && gtk4->length == 0)
    {
        ffPrintError(instance, &instance->config.themeKey, "Theme", "No themes found");
        return;
    }

    FF_STRBUF_CREATE(plasmaColorPretty);
    if(ffStrbufStartsWithIgnCase(plasmaColor, plasmaTheme))
        ffStrbufAppendNS(&plasmaColorPretty, plasmaColor->length - plasmaTheme->length, &plasmaColor->chars[plasmaTheme->length]);
    else
        ffStrbufAppend(&plasmaColorPretty, plasmaColor);

    ffStrbufTrim(&plasmaColorPretty, ' ');

    FF_STRBUF_CREATE(gtkPretty);
    ffGetGtkPretty(&gtkPretty, gtk2, gtk3, gtk4);

    if(instance->config.themeFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, &instance->config.themeKey, "Theme");

        if(plasmaTheme->length > 0)
        {
            ffStrbufWriteTo(plasmaTheme, stdout);

            if(plasmaColor->length > 0)
            {
                fputs(" (", stdout);

                if(plasmaColorPretty.length > 0)
                    ffStrbufWriteTo(&plasmaColorPretty, stdout);
                else
                    ffStrbufWriteTo(plasmaColor, stdout);

                putchar(')');
            }
        }
        else if(plasmaColor->length > 0)
        {
            if(plasmaColorPretty.length > 0)
                ffStrbufWriteTo(&plasmaColorPretty, stdout);
            else
                ffStrbufWriteTo(plasmaColor, stdout);
        }

        if(plasmaTheme->length > 0 || plasmaColor->length > 0)
        {
            fputs(" [Plasma]", stdout);

            if(gtkPretty.length > 0)
                fputs(", ", stdout);
        }

        ffStrbufPutTo(&gtkPretty, stdout);
    }
    else
    {
        ffPrintFormatString(instance, &instance->config.themeKey, "Theme", &instance->config.themeFormat, 7,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, plasmaTheme},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, plasmaColor},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &plasmaColorPretty},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, gtk2},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, gtk3},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, gtk4},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &gtkPretty}
        );
    }

    ffStrbufDestroy(&plasmaColorPretty);
    ffStrbufDestroy(&gtkPretty);
}
