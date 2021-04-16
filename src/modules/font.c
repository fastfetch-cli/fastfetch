#include "fastfetch.h"

#define FF_FONT_MODULE_NAME "Font"
#define FF_FONT_NUM_FORMAT_ARGS 17

void ffPrintFont(FFinstance* instance)
{
    ffCalculatePlasmaAndGtk(instance);

    if(instance->state.plasma.font.length == 0 && instance->state.gtk2.font.length == 0 && instance->state.gtk3.font.length == 0 && instance->state.gtk4.font.length == 0)
    {
        ffPrintError(instance, FF_FONT_MODULE_NAME, 0, &instance->config.fontKey, &instance->config.fontFormat, FF_FONT_NUM_FORMAT_ARGS, "No fonts found");
        return;
    }

    FF_STRBUF_CREATE(plasmaName);
    double plasmaSize;
    ffGetFont(instance->state.plasma.font.chars, &plasmaName, &plasmaSize);
    FF_STRBUF_CREATE(plasmaPretty);
    ffGetFontPretty(&plasmaPretty, &plasmaName, plasmaSize);

    FF_STRBUF_CREATE(gtk2Name);
    double gtk2Size;
    ffGetFont(instance->state.gtk2.font.chars, &gtk2Name, &gtk2Size);
    FF_STRBUF_CREATE(gtk2Pretty);
    ffGetFontPretty(&gtk2Pretty, &gtk2Name, gtk2Size);

    FF_STRBUF_CREATE(gtk3Name);
    double gtk3Size;
    ffGetFont(instance->state.gtk3.font.chars, &gtk3Name, &gtk3Size);
    FF_STRBUF_CREATE(gtk3Pretty);
    ffGetFontPretty(&gtk3Pretty, &gtk3Name, gtk3Size);

    FF_STRBUF_CREATE(gtk4Name);
    double gtk4Size;
    ffGetFont(instance->state.gtk4.font.chars, &gtk4Name, &gtk4Size);
    FF_STRBUF_CREATE(gtk4Pretty);
    ffGetFontPretty(&gtk4Pretty, &gtk4Name, gtk4Size);

    FF_STRBUF_CREATE(gtkPretty);
    ffGetGtkPretty(&gtkPretty, &gtk2Pretty, &gtk3Pretty, &gtk4Pretty);

    if(instance->config.fontFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_FONT_MODULE_NAME, 0, &instance->config.fontKey);
        if(instance->state.plasma.font.length > 0)
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
        ffPrintFormatString(instance, FF_FONT_MODULE_NAME, 0, &instance->config.fontKey, &instance->config.fontFormat, NULL, FF_FONT_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.plasma.font},
            {FF_FORMAT_ARG_TYPE_STRBUF, &plasmaName},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &plasmaSize},
            {FF_FORMAT_ARG_TYPE_STRBUF, &plasmaPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.gtk2.font},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk2Name},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &gtk2Size},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk2Pretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.gtk3.font},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk3Name},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &gtk3Size},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk3Pretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.gtk4.font},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk4Name},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &gtk4Size},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk4Pretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtkPretty}
        });
    }

    ffStrbufDestroy(&plasmaName);
    ffStrbufDestroy(&plasmaPretty);

    ffStrbufDestroy(&gtk2Name);
    ffStrbufDestroy(&gtk2Pretty);

    ffStrbufDestroy(&gtk3Name);
    ffStrbufDestroy(&gtk3Pretty);

    ffStrbufDestroy(&gtk4Name);
    ffStrbufDestroy(&gtk4Pretty);

    ffStrbufDestroy(&gtkPretty);
}
