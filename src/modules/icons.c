#include "fastfetch.h"

#define FF_ICONS_MODULE_NAME "Icons"
#define FF_ICONS_NUM_FORMAT_ARGS 5

void ffPrintIcons(FFinstance* instance)
{
    ffCalculatePlasmaAndGtk(instance);

    if(instance->state.plasma.icons.length == 0 && instance->state.gtk2.icons.length == 0 && instance->state.gtk3.icons.length == 0 && instance->state.gtk4.icons.length == 0)
    {
        ffPrintError(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.iconsKey, &instance->config.iconsFormat, FF_ICONS_NUM_FORMAT_ARGS, "No icons could be found");
        return;
    }

    FF_STRBUF_CREATE(gtkPretty);
    ffGetGtkPretty(&gtkPretty, &instance->state.gtk2.icons, &instance->state.gtk3.icons, &instance->state.gtk4.icons);

    if(instance->config.iconsFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.iconsKey);

        if(instance->state.plasma.icons.length > 0)
        {
            ffStrbufWriteTo(&instance->state.plasma.icons, stdout);
            fputs(" [Plasma]", stdout);

            if(gtkPretty.length > 0)
                fputs(", ", stdout);
        }

        ffStrbufPutTo(&gtkPretty, stdout);
    }
    else
    {
        ffPrintFormatString(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.iconsKey, &instance->config.iconsFormat, NULL, FF_ICONS_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.plasma.icons},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.gtk2.icons},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.gtk3.icons},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.gtk4.icons},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtkPretty}
        });
    }

    ffStrbufDestroy(&gtkPretty);
}
