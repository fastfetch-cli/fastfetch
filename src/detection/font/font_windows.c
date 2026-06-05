#include "font.h"
#include "common/windows/unicode.h"
#include "common/windows/registry.h"

#include <windows.h>

static void generateString(FFFontResult* font) {
    const char* types[] = { "Caption", "Menu", "Message", "Status" };
    for (uint32_t i = 0; i < ARRAY_SIZE(types); ++i) {
        if (i == 0 || !ffStrbufEqual(&font->fonts[i - 1], &font->fonts[i])) {
            if (i > 0) {
                ffStrbufAppendS(&font->display, "], ");
            }
            ffStrbufAppendF(&font->display, "%s [%s", font->fonts[i].chars, types[i]);
        } else {
            ffStrbufAppendS(&font->display, " / ");
            ffStrbufAppendS(&font->display, types[i]);
        }
    }
    ffStrbufAppendC(&font->display, ']');
}

const char* ffDetectFontImpl(FFFontResult* result) {
    FF_AUTO_CLOSE_FD HANDLE hKey = NULL;
    if (!ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"Control Panel\\Desktop\\WindowMetrics", &hKey, NULL)) {
        return "ffRegOpenKeyForRead(HKEY_CURRENT_USER\\Control Panel\\Desktop\\WindowMetrics) failed";
    }

    LOGFONTW fonts[4];
    FFArgBuffer fontBuffers[4] = {
        { .data = &fonts[0], .length = sizeof(fonts[0]) },
        { .data = &fonts[1], .length = sizeof(fonts[1]) },
        { .data = &fonts[2], .length = sizeof(fonts[2]) },
        { .data = &fonts[3], .length = sizeof(fonts[3]) },
    };

    if (!ffRegReadValues(hKey, 4, (FFRegValueArg[]) {
                                      FF_ARG(fontBuffers[0], L"CaptionFont"),
                                      FF_ARG(fontBuffers[1], L"MenuFont"),
                                      FF_ARG(fontBuffers[2], L"MessageFont"),
                                      FF_ARG(fontBuffers[3], L"StatusFont"),
                                  },
            NULL)) {
        return "ffRegReadValues(HKEY_CURRENT_USER\\Control Panel\\Desktop\\WindowMetrics) failed";
    }

    for (uint32_t i = 0; i < ARRAY_SIZE(fonts); ++i) {
        if (fontBuffers[i].length != sizeof(LOGFONTW)) {
            continue; // Invalid data, skip
        }

        LOGFONTW* logFont = &fonts[i];

        ffStrbufSetWS(&result->fonts[i], logFont->lfFaceName);
        if (logFont->lfHeight < 0) {
            ffStrbufAppendF(&result->fonts[i], " (%dpt)", (int) -logFont->lfHeight);
        }
    }

    generateString(result);

    return NULL;
}
