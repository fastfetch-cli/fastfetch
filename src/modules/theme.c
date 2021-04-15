#include "fastfetch.h"

#define FF_THEME_MODULE_NAME "Theme"
#define FF_THEME_NUM_FORMAT_ARGS 7

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
        ffPrintError(instance, FF_THEME_MODULE_NAME, 0, &instance->config.themeKey, &instance->config.themeFormat, FF_THEME_NUM_FORMAT_ARGS, "No themes found");
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
        ffPrintLogoAndKey(instance, FF_THEME_MODULE_NAME, 0, &instance->config.themeKey);

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
        ffPrintFormatString(instance, FF_THEME_MODULE_NAME, 0, &instance->config.themeKey, &instance->config.themeFormat, NULL, FF_THEME_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, plasmaTheme},
            {FF_FORMAT_ARG_TYPE_STRBUF, plasmaColor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &plasmaColorPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, gtk2},
            {FF_FORMAT_ARG_TYPE_STRBUF, gtk3},
            {FF_FORMAT_ARG_TYPE_STRBUF, gtk4},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtkPretty}
        });
    }

    ffStrbufDestroy(&plasmaColorPretty);
    ffStrbufDestroy(&gtkPretty);
}
