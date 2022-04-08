#include "fastfetch.h"

#define FF_THEME_MODULE_NAME "Theme"
#define FF_THEME_NUM_FORMAT_ARGS 7

void ffPrintTheme(FFinstance* instance)
{
    #ifdef __ANDROID__
        ffPrintError(instance, FF_THEME_MODULE_NAME, 0, &instance->config.themeKey, &instance->config.themeFormat, FF_THEME_NUM_FORMAT_ARGS, "Theme detection is not supported on Android");
        return;
    #endif

    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, "TTY") == 0)
    {
        ffPrintError(instance, FF_THEME_MODULE_NAME, 0, &instance->config.fontKey, &instance->config.fontFormat, FF_THEME_NUM_FORMAT_ARGS, "Theme isn't supported in TTY");
        return;
    }

    const FFPlasmaResult* plasma = ffDetectPlasma(instance);
    const FFstrbuf* gtk2 = &ffDetectGTK2(instance)->theme;
    const FFstrbuf* gtk3 = &ffDetectGTK3(instance)->theme;
    const FFstrbuf* gtk4 = &ffDetectGTK4(instance)->theme;

    if(plasma->widgetStyle.length == 0 && plasma->colorScheme.length == 0 && gtk2->length == 0 && gtk3->length == 0 && gtk4->length == 0)
    {
        ffPrintError(instance, FF_THEME_MODULE_NAME, 0, &instance->config.themeKey, &instance->config.themeFormat, FF_THEME_NUM_FORMAT_ARGS, "No themes found");
        return;
    }

    FF_STRBUF_CREATE(plasmaColorPretty);
    if(ffStrbufStartsWithIgnCase(&plasma->colorScheme, &plasma->widgetStyle))
        ffStrbufAppendNS(&plasmaColorPretty, plasma->colorScheme.length - plasma->widgetStyle.length, &plasma->colorScheme.chars[plasma->widgetStyle.length]);
    else
        ffStrbufAppend(&plasmaColorPretty, &plasma->colorScheme);

    ffStrbufTrim(&plasmaColorPretty, ' ');

    FF_STRBUF_CREATE(gtkPretty);
    ffParseGTK(&gtkPretty, gtk2, gtk3, gtk4);

    if(instance->config.themeFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_THEME_MODULE_NAME, 0, &instance->config.themeKey);

        if(plasma->widgetStyle.length > 0)
        {
            ffStrbufWriteTo(&plasma->widgetStyle, stdout);

            if(plasma->colorScheme.length > 0)
            {
                fputs(" (", stdout);

                if(plasmaColorPretty.length > 0)
                    ffStrbufWriteTo(&plasmaColorPretty, stdout);
                else
                    ffStrbufWriteTo(&plasma->colorScheme, stdout);

                putchar(')');
            }
        }
        else if(plasma->colorScheme.length > 0)
        {
            if(plasmaColorPretty.length > 0)
                ffStrbufWriteTo(&plasmaColorPretty, stdout);
            else
                ffStrbufWriteTo(&plasma->colorScheme, stdout);
        }

        if(plasma->widgetStyle.length > 0 || plasma->colorScheme.length > 0)
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
            {FF_FORMAT_ARG_TYPE_STRBUF, &plasma->widgetStyle},
            {FF_FORMAT_ARG_TYPE_STRBUF, &plasma->colorScheme},
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
