#include "common/format.h"
#include "util/textModifier.h"
#include "fastfetch.h"

#include <stdlib.h>

static void verify(const char* color, const char* expected, int lineNo)
{
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    ffOptionParseColorNoClear(color, &result);
    if (!ffStrbufEqualS(&result, expected))
    {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s: expected \"%s\", got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET, lineNo, color, expected, result.chars);
        exit(1);
    }
}

#define VERIFY(color, expected) verify((color), (expected), __LINE__)

int main(void)
{
    instance.config.display.pipe = true;
    // Initialize dummy config colors for property tests
    ffStrbufInitS(&instance.config.display.colorKeys, "94"); // light_blue
    ffStrbufInitS(&instance.config.display.colorTitle, "95"); // light_magenta

    {
        VERIFY("", "");
        VERIFY("1", "1");

        VERIFY("red", "31");
        VERIFY("light_green", "92");
        VERIFY("default", "39");
        VERIFY("blue", "34");
        VERIFY("light_cyan", "96");

        VERIFY("bold_red", "1;31");
        VERIFY("dim_light_yellow", "2;93");
        VERIFY("italic_underline_green", "3;4;32");
        VERIFY("reset_blue", "0;34"); // Reset followed by color

        VERIFY("#ff0000", "38;2;255;0;0");  // RRGGBB
        VERIFY("#0f0", "38;2;0;255;0");     // RGB
        VERIFY("#123456", "38;2;18;52;86");
        VERIFY("#abc", "38;2;170;187;204");

        VERIFY("bold_#ff00ff", "1;38;2;255;0;255");
        VERIFY("underline_#123", "4;38;2;17;34;51");

        VERIFY("\e[32m", "32"); // Direct ANSI code
        VERIFY("\e[1;94m", "1;94"); // Direct ANSI code with mode

        // Property colors (ensure dummy config colors are set)
        VERIFY("keys", "94");
        VERIFY("title", "95");
        VERIFY("bold_keys", "1;94");
    }

    // Clean up dummy config colors
    ffStrbufDestroy(&instance.config.display.colorKeys);
    ffStrbufDestroy(&instance.config.display.colorTitle);

    //Success
    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
}
