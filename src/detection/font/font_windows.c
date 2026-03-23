#include "font.h"
#include "common/windows/unicode.h"
#include "common/windows/registry.h"

#include <windows.h>

static void generateString(FFFontResult* font)
{
    const char* types[] = { "Caption", "Menu", "Message", "Status" };
    for(uint32_t i = 0; i < ARRAY_SIZE(types); ++i)
    {
        if(i == 0 || !ffStrbufEqual(&font->fonts[i - 1], &font->fonts[i]))
        {
            if(i > 0)
                ffStrbufAppendS(&font->display, "], ");
            ffStrbufAppendF(&font->display, "%s [%s", font->fonts[i].chars, types[i]);
        }
        else
        {
            ffStrbufAppendS(&font->display, " / ");
            ffStrbufAppendS(&font->display, types[i]);
        }
    }
    ffStrbufAppendC(&font->display, ']');
}

WINUSERAPI WINBOOL WINAPI ClassicSystemParametersInfoW(UINT uiAction,UINT uiParam,PVOID pvParam,UINT fWinIni);

const char* ffDetectFontImpl(FFFontResult* result)
{
    FF_AUTO_CLOSE_FD HANDLE hKey = NULL;
    if (!ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"Control Panel\\Desktop\\WindowMetrics", &hKey, NULL))
        return "ffRegOpenKeyForRead(HKEY_CURRENT_USER\\Control Panel\\Desktop\\WindowMetrics) failed";

    FF_LIST_AUTO_DESTROY CaptionFont = ffListCreate(sizeof(uint8_t));
    FF_LIST_AUTO_DESTROY MenuFont = ffListCreate(sizeof(uint8_t));
    FF_LIST_AUTO_DESTROY MessageFont = ffListCreate(sizeof(uint8_t));
    FF_LIST_AUTO_DESTROY StatusFont = ffListCreate(sizeof(uint8_t));

    if (!ffRegReadValues(hKey, 4, (FFRegValueArg[]) {
        FF_ARG(CaptionFont, L"CaptionFont"),
        FF_ARG(MenuFont, L"MenuFont"),
        FF_ARG(MessageFont, L"MessageFont"),
        FF_ARG(StatusFont, L"StatusFont"),
    }, NULL))
        return "ffRegReadValues(HKEY_CURRENT_USER\\Control Panel\\Desktop\\WindowMetrics) failed";

    FFlist* fonts[4] = { &CaptionFont, &MenuFont, &MessageFont, &StatusFont };

    for (uint32_t i = 0; i < ARRAY_SIZE(fonts); ++i)
    {
        if (fonts[i]->length < sizeof(LOGFONTW))
            continue;

        LOGFONTW* logFont = (LOGFONTW*) fonts[i]->data;

        ffStrbufSetWS(&result->fonts[i], logFont->lfFaceName);
        if (logFont->lfHeight < 0)
            ffStrbufAppendF(&result->fonts[i], " (%dpt)", (int)-logFont->lfHeight);
    }

    generateString(result);

    return NULL;
}
