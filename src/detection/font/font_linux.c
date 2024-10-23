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
        ffStrbufAppendS(&font->display, " [Qt]");

        for(uint8_t i = 1; i < ARRAY_SIZE(font->fonts); i++)
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

const char* ffDetectFontImpl(FFFontResult* result)
{
    const FFDisplayServerResult* wmde = ffConnectDisplayServer();

    if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, FF_WM_PROTOCOL_TTY) == 0)
        return "Font isn't supported in TTY";

    FFfont qt;
    ffFontInitQt(&qt, ffDetectQt()->font.chars);
    ffStrbufAppend(&result->fonts[0], &qt.pretty);
    ffFontDestroy(&qt);

    FFfont gtk2;
    ffFontInitPango(&gtk2, ffDetectGTK2()->font.chars);
    ffStrbufAppend(&result->fonts[1], &gtk2.pretty);
    ffFontDestroy(&gtk2);

    FFfont gtk3;
    ffFontInitPango(&gtk3, ffDetectGTK3()->font.chars);
    ffStrbufAppend(&result->fonts[2], &gtk3.pretty);
    ffFontDestroy(&gtk3);

    FFfont gtk4;
    ffFontInitPango(&gtk4, ffDetectGTK4()->font.chars);
    ffStrbufAppend(&result->fonts[3], &gtk4.pretty);
    ffFontDestroy(&gtk4);

    generateString(result);

    return NULL;
}
