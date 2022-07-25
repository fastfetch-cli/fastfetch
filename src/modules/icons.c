#include "fastfetch.h"
#include "common/printing.h"
#include "common/parsing.h"

#define FF_ICONS_MODULE_NAME "Icons"
#define FF_ICONS_NUM_FORMAT_ARGS 5

void ffPrintIcons(FFinstance* instance)
{
    #ifdef __ANDROID__
        ffPrintError(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.icons, "Icons detection is not supported on Android");
        return;
    #endif

    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, "TTY") == 0)
    {
        ffPrintError(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.icons, "Icons aren't supported in TTY");
        return;
    }

    const FFstrbuf* plasma = &ffDetectQt(instance)->icons;
    const FFstrbuf* gtk2 = &ffDetectGTK2(instance)->icons;
    const FFstrbuf* gtk3 = &ffDetectGTK3(instance)->icons;
    const FFstrbuf* gtk4 = &ffDetectGTK4(instance)->icons;

    if(plasma->length == 0 && gtk2->length == 0 && gtk3->length == 0 && gtk4->length == 0)
    {
        ffPrintError(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.icons, "No icons could be found");
        return;
    }

    FF_STRBUF_CREATE(gtkPretty);
    ffParseGTK(&gtkPretty, gtk2, gtk3, gtk4);

    if(instance->config.icons.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.icons.key);

        if(plasma->length > 0)
        {
            ffStrbufWriteTo(plasma, stdout);
            fputs(" [QT]", stdout);

            if(gtkPretty.length > 0)
                fputs(", ", stdout);
        }

        ffStrbufPutTo(&gtkPretty, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.icons, FF_ICONS_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, plasma},
            {FF_FORMAT_ARG_TYPE_STRBUF, gtk2},
            {FF_FORMAT_ARG_TYPE_STRBUF, gtk3},
            {FF_FORMAT_ARG_TYPE_STRBUF, gtk4},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtkPretty}
        });
    }

    ffStrbufDestroy(&gtkPretty);
}
