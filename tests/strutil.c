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
    // The plain C string helpers. These have nothing to do with wcwidth, so they run in every build.
    {
        VERIFY(!ffStrSet(nullptr));
        VERIFY(!ffStrSet(""));
        VERIFY(!ffStrSet(" "));
        VERIFY(!ffStrSet("  \t \n"));
        VERIFY(ffStrSet("a"));
        VERIFY(ffStrSet(" a"));
        VERIFY(ffStrSet("a "));
    }

    {
        VERIFY(ffStrStartsWith("", ""));
        VERIFY(ffStrStartsWith("abc", ""));
        VERIFY(ffStrStartsWith("abc", "a"));
        VERIFY(ffStrStartsWith("abc", "abc"));
        VERIFY(!ffStrStartsWith("abc", "abcd")); // the needle is longer than the haystack
        VERIFY(!ffStrStartsWith("abc", "b"));
        VERIFY(!ffStrStartsWith("", "a"));
        VERIFY(!ffStrStartsWith("Abc", "abc"));

        VERIFY(ffStrStartsWithIgnCase("Abc", "abc"));
        VERIFY(ffStrStartsWithIgnCase("ABC", "abc"));
        VERIFY(ffStrStartsWithIgnCase("abc", ""));
        VERIFY(!ffStrStartsWithIgnCase("Abc", "abd"));
    }

    {
        VERIFY(ffStrEndsWith("", ""));
        VERIFY(ffStrEndsWith("abc", ""));
        VERIFY(ffStrEndsWith("abc", "c"));
        VERIFY(ffStrEndsWith("abc", "abc"));
        VERIFY(!ffStrEndsWith("abc", "abcd")); // the needle is longer than the haystack
        VERIFY(!ffStrEndsWith("abc", "b"));
        VERIFY(!ffStrEndsWith("", "a"));
        VERIFY(!ffStrEndsWith("abc", "Abc"));

        VERIFY(ffStrEndsWithIgnCase("abc", "BC"));
        VERIFY(ffStrEndsWithIgnCase("abc", ""));
        VERIFY(!ffStrEndsWithIgnCase("abc", "BD"));
    }

    {
        VERIFY(ffStrEquals("", ""));
        VERIFY(ffStrEquals("abc", "abc"));
        VERIFY(!ffStrEquals("abc", "abcd"));
        VERIFY(!ffStrEquals("abc", "ABC"));

        VERIFY(ffStrEqualsIgnCase("abc", "ABC"));
        VERIFY(ffStrEqualsIgnCase("", ""));
        VERIFY(!ffStrEqualsIgnCase("abc", "abd"));

        VERIFY(ffStrContains("abc", ""));
        VERIFY(ffStrContains("abc", "b"));
        VERIFY(ffStrContains("abc", "abc"));
        VERIFY(!ffStrContains("abc", "d"));

        VERIFY(ffStrContainsIgnCase("abc", "B"));
        VERIFY(!ffStrContainsIgnCase("abc", "D"));
        // `strcasestr` reports an empty needle as a match, but the `StrStrIA` it is aliased to on
        // Windows reports no match, so on Windows this disagrees with `ffStrContains`.
#ifndef _WIN32
        VERIFY(ffStrContainsIgnCase("abc", ""));
#endif

        VERIFY(ffStrContainsC("abc", 'b'));
        VERIFY(!ffStrContainsC("abc", 'd'));
        VERIFY(ffStrContainsC("abc", '\0')); // `strchr` reports the terminator as a match
    }

    {
        VERIFY(ffCharIsEnglishAlphabet('a'));
        VERIFY(ffCharIsEnglishAlphabet('z'));
        VERIFY(ffCharIsEnglishAlphabet('A'));
        VERIFY(ffCharIsEnglishAlphabet('Z'));
        VERIFY(!ffCharIsEnglishAlphabet('0'));
        VERIFY(!ffCharIsEnglishAlphabet('_'));
        VERIFY(!ffCharIsEnglishAlphabet(' '));

        VERIFY(ffCharIsDigit('0'));
        VERIFY(ffCharIsDigit('9'));
        VERIFY(!ffCharIsDigit('/')); // one below '0'
        VERIFY(!ffCharIsDigit(':')); // one above '9'
        VERIFY(!ffCharIsDigit('a'));

        VERIFY(ffCharIsHexDigit('0'));
        VERIFY(ffCharIsHexDigit('9'));
        VERIFY(ffCharIsHexDigit('a'));
        VERIFY(ffCharIsHexDigit('F'));
        VERIFY(!ffCharIsHexDigit('g'));
        VERIFY(!ffCharIsHexDigit('G'));
        VERIFY(!ffCharIsHexDigit('/'));
        VERIFY(!ffCharIsHexDigit('@'));
    }

    {
        VERIFY(ffHexCharToInt('0') == 0);
        VERIFY(ffHexCharToInt('9') == 9);
        VERIFY(ffHexCharToInt('a') == 10);
        VERIFY(ffHexCharToInt('f') == 15);
        VERIFY(ffHexCharToInt('A') == 10);
        VERIFY(ffHexCharToInt('F') == 15);
        VERIFY(ffHexCharToInt('g') == -1);
        VERIFY(ffHexCharToInt('G') == -1);
        VERIFY(ffHexCharToInt('/') == -1); // one below '0'
        VERIFY(ffHexCharToInt(':') == -1); // one above '9'
        VERIFY(ffHexCharToInt('@') == -1); // one below 'A'
        VERIFY(ffHexCharToInt('`') == -1); // one below 'a'
        VERIFY(ffHexCharToInt(' ') == -1);
    }

    // ffStrCopy never writes more than `dstBufSiz` bytes and always terminates. It returns a pointer
    // to the end of the copy, so consecutive calls can be chained.
    {
        char buffer[8];

        memset(buffer, 'X', sizeof(buffer));
        char* end = ffStrCopy(buffer, "abc", sizeof(buffer));
        VERIFY(ffStrEquals(buffer, "abc"));
        VERIFY(end == buffer + 3);

        memset(buffer, 'X', sizeof(buffer));
        end = ffStrCopy(buffer, "abcdef", sizeof(buffer));
        VERIFY(ffStrEquals(buffer, "abcdef"));
        VERIFY(end == buffer + 6);

        // Filling the buffer exactly still leaves room for the terminator
        char exact[4];
        memset(exact, 'X', sizeof(exact));
        end = ffStrCopy(exact, "abc", sizeof(exact));
        VERIFY(ffStrEquals(exact, "abc"));
        VERIFY(end == exact + 3);

        // One byte short: the copy is truncated to `dstBufSiz - 1` bytes
        char truncated[3];
        memset(truncated, 'X', sizeof(truncated));
        end = ffStrCopy(truncated, "abcdef", sizeof(truncated));
        VERIFY(ffStrEquals(truncated, "ab"));
        VERIFY(end == truncated + 2);

        // A one byte buffer holds nothing but the terminator
        char single[1];
        single[0] = 'X';
        end = ffStrCopy(single, "abc", sizeof(single));
        VERIFY(single[0] == '\0');
        VERIFY(end == single);

        // A zero sized buffer is left completely alone
        char untouched[1] = { 'X' };
        end = ffStrCopy(untouched, "abc", 0);
        VERIFY(untouched[0] == 'X');
        VERIFY(end == untouched);

        VERIFY(ffStrCopy(nullptr, "abc", 8) == nullptr);
    }

    // FF_STR stringifies its argument
    {
        VERIFY(ffStrEquals(FF_STR(1), "1"));
        VERIFY(ffStrEquals(FF_STR(123), "123"));
    }

    #if FF_ENABLE_WCWIDTH
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
        VERIFY(bytes == 3);
        VERIFY(width == 2);
    }

    {
        const char* combining = "\xCC\x81"; // ◌́ U+0301
        uint8_t width = 0;
        uint8_t bytes = ffUtf8CharLenWidth(combining, 2, &width);
        VERIFY(bytes == 2);
        VERIFY(width == 0); // Should be 1 since there's no base character
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
                            "\xE6\x96\x87" // 文 U+6587
                            "B";
        VERIFY(ffUtf8StrWidth(mixed, 5) == 4);
    }

    {
        const char* combining = "A\xCC\x81"; // Á
        VERIFY(ffUtf8StrWidth(combining, 3) == 1);
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
        const char* emoji = "\xF0\x9F\x98\x80"; // U+1F600 😀
        uint8_t width = 0;
        uint8_t bytes = ffUtf8CharLenWidth(emoji, 4, &width);
        VERIFY(bytes == 4);
        VERIFY(width == 2);
    }

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    #else
    puts("\033[33mwcwidth tests skipped because wcwidth support is disabled." FASTFETCH_TEXT_MODIFIER_RESET);
    #endif
    return 0;
}
