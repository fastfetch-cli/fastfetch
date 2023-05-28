#include "common/font.h"
#include "common/parsing.h"
#include "detection/displayserver/displayserver.h"
#include "detection/gtk_qt/gtk_qt.h"
#include "font.h"

static void generateString(FFFontResult* font)
{
    if(font->fonts[0].length > 0)
    {
        ffStrbufAppend(&font->display, &font->fonts[0]);
        ffStrbufAppendS(&font->display, " [QT]");

        for(uint8_t i = 1; i < sizeof(font->fonts) / sizeof(font->fonts[0]); i++)
        {
            if(font->fonts[i].length > 0)
            {
                ffStrbufAppendS(&font->display, ", ");
                break;
            }
        }
    }

    ffParseGTK(&font->display, &font->fonts[1], &font->fonts[2], &font->fonts[3]);
}

void ffDetectFontImpl(const FFinstance* instance, FFFontResult* result)
{
    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, FF_WM_PROTOCOL_TTY) == 0)
    {
        ffStrbufAppendS(&result->error, "Font isn't supported in TTY");
        return;
    }

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

    generateString(result);
}
