#include "common/strutil.h"
#include "common/textModifier.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void verify(bool expression, const char* expressionStr, int lineNo) {
    if (expression) {
        return;
    }

    fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s\n" FASTFETCH_TEXT_MODIFIER_RESET, lineNo, expressionStr);
    exit(1);
}

#define VERIFY(expression) verify((expression), #expression, __LINE__)

int main(void) {
    {
        uint8_t width = 255;
        uint8_t bytes = ffUtf8CharLenWidth("", 0, &width);
        VERIFY(bytes == 0);
        VERIFY(width == 0);
    }

    {
        uint8_t width = 0;
        uint8_t bytes = ffUtf8CharLenWidth("A", 1, &width);
        VERIFY(bytes == 1);
        VERIFY(width == 1);
    }

    {
        const char* ch = "\xE6\x96\x87"; // 文 U+6587
        uint8_t width = 0;
        uint8_t bytes = ffUtf8CharLenWidth(ch, 3, &width);
        int expected = mk_wcwidth(0x6587);
        VERIFY(bytes == 3);
        VERIFY(width == (uint8_t) (expected < 0 ? 0 : expected));
    }

    {
        const char* combining = "\xCC\x81"; // U+0301
        uint8_t width = 0;
        uint8_t bytes = ffUtf8CharLenWidth(combining, 2, &width);
        int expected = mk_wcwidth(0x0301);
        VERIFY(bytes == 2);
        VERIFY(width == (uint8_t) (expected < 0 ? 0 : expected));
    }

    {
        uint8_t width = 0;
        uint8_t bytes = ffUtf8CharLenWidth("\xE6\x96\x87", 1, &width); // truncated
        VERIFY(bytes == 1);
        VERIFY(width == 1);
    }

    {
        uint8_t width = 0;
        uint8_t bytes = ffUtf8CharLenWidth("\xE6"
                                           "A",
            2,
            &width); // invalid continuation
        VERIFY(bytes == 1);
        VERIFY(width == 1);
    }

    {
        VERIFY(ffUtf8StrWidth("abc", 3) == 3);
    }

    {
        const char* mixed = "A"
                            "\xE6\x96\x87"
                            "B";
        int wide = mk_wcwidth(0x6587);
        uint32_t expected = 2 + (uint32_t) (wide < 0 ? 0 : wide);
        VERIFY(ffUtf8StrWidth(mixed, 5) == expected);
    }

    {
        const char* combining = "A\xCC\x81";
        int wCombining = mk_wcwidth(0x0301);
        uint32_t expected = 1 + (uint32_t) (wCombining < 0 ? 0 : wCombining);
        VERIFY(ffUtf8StrWidth(combining, 3) == expected);
    }

    {
        VERIFY(ffUtf8StrWidth("\xE6"
                              "A",
                   2) == 2);
    }

    {
        VERIFY(ffUtf8StrWidth("", 0) == 0);
        VERIFY(ffUtf8StrWidth("A\0B", 3) == 1);
    }

    {
        const char* combining = "\xCC\x81";
        int wCombining = mk_wcwidth(0x0301);
        uint32_t normalized = (uint32_t) (wCombining < 0 ? 0 : wCombining);
        uint32_t expected = normalized > 0 ? normalized : 2;
        VERIFY(ffUtf8StrWidth(combining, 2) == expected);
    }

    {
        const char* emoji = "\xF0\x9F\x98\x80"; // U+1F600 😀
        uint8_t width = 0;
        uint8_t bytes = ffUtf8CharLenWidth(emoji, 4, &width);
        VERIFY(bytes == 4);
        VERIFY(width == 2); // Most emoji are double-width
    }

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
