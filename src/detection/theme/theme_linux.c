#include "theme.h"
#include "common/parsing.h"
#include "detection/gtk_qt/gtk_qt.h"
#include "detection/displayserver/displayserver.h"

const char* ffDetectTheme(const FFinstance* instance, FFstrbuf* result)
{
    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, FF_WM_PROTOCOL_TTY) == 0)
        return "Theme isn't supported in TTY";

    const FFQtResult* plasma = ffDetectQt(instance);
    const FFstrbuf* gtk2 = &ffDetectGTK2(instance)->theme;
    const FFstrbuf* gtk3 = &ffDetectGTK3(instance)->theme;
    const FFstrbuf* gtk4 = &ffDetectGTK4(instance)->theme;

    if(plasma->widgetStyle.length == 0 && plasma->colorScheme.length == 0 && gtk2->length == 0 && gtk3->length == 0 && gtk4->length == 0)
        return "No themes found";

    FF_STRBUF_AUTO_DESTROY plasmaColorPretty = ffStrbufCreate();
    if(ffStrbufStartsWithIgnCase(&plasma->colorScheme, &plasma->widgetStyle))
        ffStrbufAppendNS(&plasmaColorPretty, plasma->colorScheme.length - plasma->widgetStyle.length, &plasma->colorScheme.chars[plasma->widgetStyle.length]);
    else
        ffStrbufAppend(&plasmaColorPretty, &plasma->colorScheme);

    ffStrbufTrim(&plasmaColorPretty, ' ');

    FF_STRBUF_AUTO_DESTROY gtkPretty = ffStrbufCreate();
    ffParseGTK(&gtkPretty, gtk2, gtk3, gtk4);

    if(plasma->widgetStyle.length > 0)
    {
        ffStrbufAppend(result, &plasma->widgetStyle);

        if(plasma->colorScheme.length > 0)
        {
            ffStrbufAppendS(result, " (");

            if(plasmaColorPretty.length > 0)
                ffStrbufAppend(result, &plasmaColorPretty);
            else
                ffStrbufAppend(result, &plasma->colorScheme);

            ffStrbufAppendC(result, ')');
        }
    }
    else if(plasma->colorScheme.length > 0)
    {
        if(plasmaColorPretty.length > 0)
            ffStrbufAppend(result, &plasmaColorPretty);
        else
            ffStrbufAppend(result, &plasma->colorScheme);
    }

    if(plasma->widgetStyle.length > 0 || plasma->colorScheme.length > 0)
    {
        ffStrbufAppendS(result, " [QT]");

        if(gtkPretty.length > 0)
            ffStrbufAppendS(result, ", ");
    }

    ffStrbufAppend(result, &gtkPretty);

    return NULL;
}
