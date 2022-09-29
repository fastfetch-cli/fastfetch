#include "detection/qt.h"
#include "detection/gtk.h"
#include "common/font.h"
#include "font.h"

void ffDetectFontImpl(const FFinstance* instance, FFFontResult* result)
{
    FFfont qt;
    ffFontInitQt(&qt, ffDetectQt(instance)->font.chars);
    ffStrbufAppend(&result->fonts[0], &qt.pretty);
    ffFontDestroy(&qt);

    FFfont gtk2;
    ffFontInitPango(&gtk2, ffDetectGTK2(instance)->font.chars);
    ffStrbufAppend(&result->fonts[1], &gtk2.pretty);
    ffFontDestroy(&gtk2);

    FFfont gtk3;
    ffFontInitPango(&gtk3, ffDetectGTK3(instance)->font.chars);
    ffStrbufAppend(&result->fonts[2], &gtk3.pretty);
    ffFontDestroy(&gtk3);

    FFfont gtk4;
    ffFontInitPango(&gtk4, ffDetectGTK4(instance)->font.chars);
    ffStrbufAppend(&result->fonts[3], &gtk4.pretty);
    ffFontDestroy(&gtk4);
}
