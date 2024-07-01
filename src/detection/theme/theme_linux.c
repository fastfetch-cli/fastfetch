#include "theme.h"
#include "common/parsing.h"
#include "detection/gtk_qt/gtk_qt.h"
#include "detection/displayserver/displayserver.h"

const char* ffDetectTheme(FFThemeResult* result)
{
    const FFDisplayServerResult* wmde = ffConnectDisplayServer();

    if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, FF_WM_PROTOCOL_TTY) == 0)
        return "Theme isn't supported in TTY";

    const FFQtResult* plasma = ffDetectQt();
    const FFstrbuf* gtk2 = &ffDetectGTK2()->theme;
    const FFstrbuf* gtk3 = &ffDetectGTK3()->theme;
    const FFstrbuf* gtk4 = &ffDetectGTK4()->theme;

    if(plasma->widgetStyle.length == 0 && plasma->colorScheme.length == 0 && gtk2->length == 0 && gtk3->length == 0 && gtk4->length == 0)
        return "No themes found";

    ffParseGTK(&result->theme2, gtk2, gtk3, gtk4);

    FF_STRBUF_AUTO_DESTROY plasmaColorPretty = ffStrbufCreate();
    if(ffStrbufStartsWithIgnCase(&plasma->colorScheme, &plasma->widgetStyle))
        ffStrbufAppendNS(&plasmaColorPretty, plasma->colorScheme.length - plasma->widgetStyle.length, &plasma->colorScheme.chars[plasma->widgetStyle.length]);
    else
        ffStrbufAppend(&plasmaColorPretty, &plasma->colorScheme);

    ffStrbufTrim(&plasmaColorPretty, ' ');

    if(plasma->widgetStyle.length > 0)
    {
        ffStrbufAppend(&result->theme1, &plasma->widgetStyle);

        if(plasma->colorScheme.length > 0)
        {
            ffStrbufAppendS(&result->theme1, " (");

            if(plasmaColorPretty.length > 0)
                ffStrbufAppend(&result->theme1, &plasmaColorPretty);
            else
                ffStrbufAppend(&result->theme1, &plasma->colorScheme);

            ffStrbufAppendC(&result->theme1, ')');
        }
    }
    else if(plasma->colorScheme.length > 0)
    {
        if(plasmaColorPretty.length > 0)
            ffStrbufAppend(&result->theme1, &plasmaColorPretty);
        else
            ffStrbufAppend(&result->theme1, &plasma->colorScheme);
    }

    if(plasma->widgetStyle.length > 0 || plasma->colorScheme.length > 0)
        ffStrbufAppendS(&result->theme1, " [Qt]");

    return NULL;
}
