#include "icons.h"
#include "common/parsing.h"
#include "detection/gtk_qt/gtk_qt.h"
#include "detection/displayserver/displayserver.h"

const char* ffDetectIcons(FFIconsResult* result)
{
    const FFDisplayServerResult* wmde = ffConnectDisplayServer();

    if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, FF_WM_PROTOCOL_TTY) == 0)
        return "Icons aren't supported in TTY";

    const FFstrbuf* plasma = &ffDetectQt()->icons;
    const FFstrbuf* gtk2 = &ffDetectGTK2()->icons;
    const FFstrbuf* gtk3 = &ffDetectGTK3()->icons;
    const FFstrbuf* gtk4 = &ffDetectGTK4()->icons;

    if(plasma->length == 0 && gtk2->length == 0 && gtk3->length == 0 && gtk4->length == 0)
        return "No icons could be found";

    ffParseGTK(&result->icons2, gtk2, gtk3, gtk4);

    if(plasma->length > 0)
    {
        ffStrbufAppend(&result->icons1, plasma);
        ffStrbufAppendS(&result->icons1, " [Qt]");
    }

    return NULL;
}
