#include "fastfetch.h"
#include "common/font.h"
#include "common/printing.h"
#include "common/parsing.h"
#include "detection/qt.h"

#define FF_FONT_MODULE_NAME "Font"
#define FF_FONT_NUM_FORMAT_ARGS 21

void ffPrintFont(FFinstance* instance)
{
    #ifdef __ANDROID__
        ffPrintError(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font, "Font detection is not supported on Android");
        return;
    #endif

    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, "TTY") == 0)
    {
        ffPrintError(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font, "Font isn't supported in TTY");
        return;
    }

    const FFstrbuf* plasmaRaw = &ffDetectQt(instance)->font;
    const FFstrbuf* gtk2Raw = &ffDetectGTK2(instance)->font;
    const FFstrbuf* gtk3Raw = &ffDetectGTK3(instance)->font;
    const FFstrbuf* gtk4Raw = &ffDetectGTK4(instance)->font;

    if(plasmaRaw->length == 0 && gtk2Raw->length == 0 && gtk3Raw->length == 0 && gtk4Raw->length == 0)
    {
        ffPrintError(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font, "No fonts found");
        return;
    }

    FFfont plasma;
    ffFontInitQt(&plasma, plasmaRaw->chars);

    FFfont gtk2;
    ffFontInitPango(&gtk2, gtk2Raw->chars);

    FFfont gtk3;
    ffFontInitPango(&gtk3, gtk3Raw->chars);

    FFfont gtk4;
    ffFontInitPango(&gtk4, gtk4Raw->chars);

    FFstrbuf gtk;
    ffStrbufInitA(&gtk, 64);
    ffParseGTK(&gtk, &gtk2.pretty, &gtk3.pretty, &gtk4.pretty);

    if(instance->config.font.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font.key);
        if(plasma.pretty.length > 0)
        {
            ffStrbufWriteTo(&plasma.pretty, stdout);
            fputs(" [QT]", stdout);

            if(gtk.length > 0)
                fputs(", ", stdout);
        }
        ffStrbufPutTo(&gtk, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font, FF_FONT_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, plasmaRaw},
            {FF_FORMAT_ARG_TYPE_STRBUF, &plasma.name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &plasma.size},
            {FF_FORMAT_ARG_TYPE_LIST,   &plasma.styles},
            {FF_FORMAT_ARG_TYPE_STRBUF, &plasma.pretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, gtk2Raw},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk2.name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk2.size},
            {FF_FORMAT_ARG_TYPE_LIST,   &gtk2.styles},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk2.pretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, gtk3Raw},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk3.name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk3.size},
            {FF_FORMAT_ARG_TYPE_LIST,   &gtk3.styles},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk3.pretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, gtk4Raw},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk4.name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk4.size},
            {FF_FORMAT_ARG_TYPE_LIST,   &gtk4.styles},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk4.pretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &gtk}
        });
    }

    ffFontDestroy(&plasma);
    ffFontDestroy(&gtk2);
    ffFontDestroy(&gtk3);
    ffFontDestroy(&gtk4);
    ffStrbufDestroy(&gtk);
}
