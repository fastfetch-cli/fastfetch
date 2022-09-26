#include "detection/qt.h"
#include "detection/gtk.h"
#include "common/parsing.h"
#include "common/font.h"
#include "font.h"

const char* ffDetectFontImpl(FFinstance* instance, FFlist* result)
{
    const FFstrbuf* qtRaw = &ffDetectQt(instance)->font;
    const FFstrbuf* gtk2Raw = &ffDetectGTK2(instance)->font;
    const FFstrbuf* gtk3Raw = &ffDetectGTK3(instance)->font;
    const FFstrbuf* gtk4Raw = &ffDetectGTK4(instance)->font;

    if(qtRaw->length > 0)
    {
        FFfont qt;
        ffFontInitQt(&qt, qtRaw->chars);

        FFFontResult* resultQt = (FFFontResult *)ffListAdd(result);
        resultQt->type = "QT";
        ffStrbufInitCopy(&resultQt->fontPretty, &qt.pretty);

        ffFontDestroy(&qt);
    }

    if(gtk2Raw->length > 0 || gtk3Raw->length > 0 || gtk4Raw->length > 0)
    {
        FFfont gtk2;
        ffFontInitPango(&gtk2, gtk2Raw->chars);

        FFfont gtk3;
        ffFontInitPango(&gtk3, gtk3Raw->chars);

        FFfont gtk4;
        ffFontInitPango(&gtk4, gtk4Raw->chars);

        FFFontResult* resultGtk = (FFFontResult *)ffListAdd(result);
        resultGtk->type = "GTK";
        ffStrbufInit(&resultGtk->fontPretty);
        ffParseGTK(&resultGtk->fontPretty, &gtk2.pretty, &gtk3.pretty, &gtk4.pretty);

        ffFontDestroy(&gtk2);
        ffFontDestroy(&gtk3);
        ffFontDestroy(&gtk4);
    }

    if(result->length == 0)
        return "No fonts found";

    return NULL;
}
