#include "icons.h"
#include "common/parsing.h"
#include "detection/gtk_qt/gtk_qt.h"
#include "detection/displayserver/displayserver.h"

const char* ffDetectIcons(const FFinstance* instance, FFstrbuf* result)
{
    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, FF_WM_PROTOCOL_TTY) == 0)
        return "Icons aren't supported in TTY";

    const FFstrbuf* plasma = &ffDetectQt(instance)->icons;
    const FFstrbuf* gtk2 = &ffDetectGTK2(instance)->icons;
    const FFstrbuf* gtk3 = &ffDetectGTK3(instance)->icons;
    const FFstrbuf* gtk4 = &ffDetectGTK4(instance)->icons;

    if(plasma->length == 0 && gtk2->length == 0 && gtk3->length == 0 && gtk4->length == 0)
        return "No icons could be found";

    FF_STRBUF_AUTO_DESTROY gtkPretty;
    ffStrbufInit(&gtkPretty);
    ffParseGTK(&gtkPretty, gtk2, gtk3, gtk4);

    if(plasma->length > 0)
    {
        ffStrbufAppend(result, plasma);
        ffStrbufAppendS(result, " [QT]");

        if(gtkPretty.length > 0)
            ffStrbufAppendS(result, ", ");
    }
    ffStrbufAppend(result, &gtkPretty);

    return NULL;
}
