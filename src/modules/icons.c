#include "fastfetch.h"

#define FF_ICONS_MODULE_NAME "Icons"
#define FF_ICONS_NUM_FORMAT_ARGS 5

void ffPrintIcons(FFinstance* instance)
{
    const FFWMDEResult* wmde = ffDetectWMDE(instance);

    if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, "TTY") == 0)
    {
        ffPrintError(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.fontKey, &instance->config.fontFormat, FF_ICONS_NUM_FORMAT_ARGS, "Icons aren't supported in TTY");
        return;
    }

    const FFstrbuf* plasma = &ffDetectPlasma(instance)->icons;
    const FFstrbuf* gtk2 = &ffDetectGTK2(instance)->icons;
    const FFstrbuf* gtk3 = &ffDetectGTK3(instance)->icons;
    const FFstrbuf* gtk4 = &ffDetectGTK4(instance)->icons;

    if(plasma->length == 0 && gtk2->length == 0 && gtk3->length == 0 && gtk4->length == 0)
    {
        ffPrintError(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.iconsKey, &instance->config.iconsFormat, FF_ICONS_NUM_FORMAT_ARGS, "No icons could be found");
        return;
    }

    FF_STRBUF_CREATE(gtkPretty);
    ffGetGtkPretty(&gtkPretty, gtk2, gtk3, gtk4);

    if(instance->config.iconsFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.iconsKey);

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
        ffPrintFormatString(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.iconsKey, &instance->config.iconsFormat, NULL, FF_ICONS_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, plasma},
            {FF_FORMAT_ARG_TYPE_STRBUF, gtk2},
            {FF_FORMAT_ARG_TYPE_STRBUF, gtk3},
            {FF_FORMAT_ARG_TYPE_STRBUF, gtk4},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtkPretty}
        });
    }

    ffStrbufDestroy(&gtkPretty);
}
