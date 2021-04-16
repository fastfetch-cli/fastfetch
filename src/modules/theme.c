#include "fastfetch.h"

#define FF_THEME_MODULE_NAME "Theme"
#define FF_THEME_NUM_FORMAT_ARGS 7

void ffPrintTheme(FFinstance* instance)
{
    ffCalculatePlasmaAndGtk(instance);

    if(instance->state.plasma.widgetStyle.length == 0 && instance->state.plasma.colorScheme.length == 0 && instance->state.gtk2.theme.length == 0 && instance->state.gtk3.theme.length == 0 && instance->state.gtk4.theme.length == 0)
    {
        ffPrintError(instance, FF_THEME_MODULE_NAME, 0, &instance->config.themeKey, &instance->config.themeFormat, FF_THEME_NUM_FORMAT_ARGS, "No themes found");
        return;
    }

    FF_STRBUF_CREATE(plasmaColorPretty);
    if(ffStrbufStartsWithIgnCase(&instance->state.plasma.colorScheme, &instance->state.plasma.widgetStyle))
        ffStrbufAppendNS(&plasmaColorPretty, instance->state.plasma.colorScheme.length - instance->state.plasma.widgetStyle.length, &instance->state.plasma.colorScheme.chars[instance->state.plasma.widgetStyle.length]);
    else
        ffStrbufAppend(&plasmaColorPretty, &instance->state.plasma.colorScheme);

    ffStrbufTrim(&plasmaColorPretty, ' ');

    FF_STRBUF_CREATE(gtkPretty);
    ffGetGtkPretty(&gtkPretty, &instance->state.gtk2.theme, &instance->state.gtk3.theme, &instance->state.gtk4.theme);

    if(instance->config.themeFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_THEME_MODULE_NAME, 0, &instance->config.themeKey);

        if(instance->state.plasma.widgetStyle.length > 0)
        {
            ffStrbufWriteTo(&instance->state.plasma.widgetStyle, stdout);

            if(instance->state.plasma.colorScheme.length > 0)
            {
                fputs(" (", stdout);

                if(plasmaColorPretty.length > 0)
                    ffStrbufWriteTo(&plasmaColorPretty, stdout);
                else
                    ffStrbufWriteTo(&instance->state.plasma.colorScheme, stdout);

                putchar(')');
            }
        }
        else if(instance->state.plasma.colorScheme.length > 0)
        {
            if(plasmaColorPretty.length > 0)
                ffStrbufWriteTo(&plasmaColorPretty, stdout);
            else
                ffStrbufWriteTo(&instance->state.plasma.colorScheme, stdout);
        }

        if(instance->state.plasma.widgetStyle.length > 0 || instance->state.plasma.colorScheme.length > 0)
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
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.plasma.widgetStyle},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.plasma.colorScheme},
            {FF_FORMAT_ARG_TYPE_STRBUF, &plasmaColorPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.gtk2.theme},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.gtk3.theme},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.gtk4.theme},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtkPretty}
        });
    }

    ffStrbufDestroy(&plasmaColorPretty);
    ffStrbufDestroy(&gtkPretty);
}
