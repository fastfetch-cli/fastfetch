#include "common/option.h"
#include "common/percent.h"
#include "common/textModifier.h"
#include "fastfetch.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>

static FFModuleArgs module;

static void verifyNum(double percent, FFPercentageModuleConfig config, bool parentheses, const char* expected, int lineNo) {
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    ffPercentAppendNum(&result, percent, config, parentheses, &module);

    if (!ffStrbufEqualS(&result, expected)) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffPercentAppendNum(%f, {%u, %u, %u}, %s): expected \"%s\", got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET,
            lineNo, percent, config.green, config.yellow, (unsigned) config.type, parentheses ? "true" : "false", expected, result.chars);
        exit(1);
    }
}

#define VERIFY_NUM(percent, config, parentheses, expected) verifyNum((percent), (config), (parentheses), (expected), __LINE__)

static void verifyBar(double percent, FFPercentageModuleConfig config, const char* expected, int lineNo) {
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    ffPercentAppendBar(&result, percent, config, &module);

    if (!ffStrbufEqualS(&result, expected)) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffPercentAppendBar(%f, {%u, %u, %u}): expected \"%s\", got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET,
            lineNo, percent, config.green, config.yellow, (unsigned) config.type, expected, result.chars);
        exit(1);
    }
}

#define VERIFY_BAR(percent, config, expected) verifyBar((percent), (config), (expected), __LINE__)

int main(void) {
    // Use the real defaults rather than a hand built config, so a changed default is caught here too
    ffOptionsInitDisplay(&instance.config.display);
    FFOptionsDisplay* options = &instance.config.display;
    ffOptionInitModuleArg(&module, "");

    // Pin the threshold colors: the yellow and red defaults depend on whether the terminal is
    // detected as a light theme, which is not something a test should depend on.
    ffStrbufSetS(&options->percentColorGreen, "32");
    ffStrbufSetS(&options->percentColorYellow, "33");
    ffStrbufSetS(&options->percentColorRed, "31");

    // The plain number. `pipe` disables the color, which is what the first group of cases checks.
    {
        options->pipe = true;
        const FFPercentageModuleConfig config = { .green = 50, .yellow = 80, .type = FF_PERCENTAGE_TYPE_NUM_BIT | FF_PERCENTAGE_TYPE_NUM_COLOR_BIT };

        VERIFY_NUM(0, config, false, "0%");
        VERIFY_NUM(50, config, false, "50%");
        VERIFY_NUM(100, config, false, "100%");
        VERIFY_NUM(99.9, config, false, "100%"); // the default is zero digits, so this rounds

        // An unset percentage prints as a bare dash, without a percent sign
        VERIFY_NUM(-DBL_MAX, config, false, "-");

        // Parentheses wrap the whole thing, color included
        VERIFY_NUM(0, config, true, "(0%)");
        VERIFY_NUM(-DBL_MAX, config, true, "(-)");
    }

    // percentNdigits
    {
        options->pipe = true;
        options->percentNdigits = 2;
        const FFPercentageModuleConfig config = { .green = 50, .yellow = 80, .type = FF_PERCENTAGE_TYPE_NUM_BIT };

        VERIFY_NUM(0, config, false, "0.00%");
        VERIFY_NUM(50, config, false, "50.00%");
        VERIFY_NUM(33.333, config, false, "33.33%");
        VERIFY_NUM(100, config, false, "100.00%");
        VERIFY_NUM(-DBL_MAX, config, false, "-");

        options->percentNdigits = 0;
    }

    // percentWidth pads on the left, and the percent sign is not part of the padded field
    {
        options->pipe = true;
        options->percentWidth = 5;
        const FFPercentageModuleConfig config = { .green = 50, .yellow = 80, .type = FF_PERCENTAGE_TYPE_NUM_BIT };

        VERIFY_NUM(0, config, false, "    0%");
        VERIFY_NUM(50, config, false, "   50%");
        VERIFY_NUM(100, config, false, "  100%");

        options->percentWidth = 0;
    }

    // Only `always` adds a space before the percent sign
    {
        options->pipe = true;
        const FFPercentageModuleConfig config = { .green = 50, .yellow = 80, .type = FF_PERCENTAGE_TYPE_NUM_BIT };

        options->percentSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_ALWAYS;
        VERIFY_NUM(50, config, false, "50 %");

        options->percentSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_NEVER;
        VERIFY_NUM(50, config, false, "50%");

        options->percentSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_DEFAULT;
        VERIFY_NUM(50, config, false, "50%");
    }

    // The color is picked by comparing against the thresholds, and each threshold belongs to the
    // lower band: exactly `green` is green, exactly `yellow` is yellow.
    {
        options->pipe = false;
        const FFPercentageModuleConfig config = { .green = 50, .yellow = 80, .type = FF_PERCENTAGE_TYPE_NUM_COLOR_BIT };

        VERIFY_NUM(0, config, false, "\e[32m0%\e[m");
        VERIFY_NUM(50, config, false, "\e[32m50%\e[m");
        VERIFY_NUM(51, config, false, "\e[33m51%\e[m");
        VERIFY_NUM(79, config, false, "\e[33m79%\e[m");
        VERIFY_NUM(80, config, false, "\e[33m80%\e[m");
        VERIFY_NUM(81, config, false, "\e[31m81%\e[m");
        VERIFY_NUM(100, config, false, "\e[31m100%\e[m");

        VERIFY_NUM(-DBL_MAX, config, false, "\e[90m-\e[m");
        VERIFY_NUM(-DBL_MAX, config, true, "(\e[90m-\e[m)");
    }

    // When `green > yellow` the bands are inverted, so a low value is the alarming one
    {
        options->pipe = false;
        const FFPercentageModuleConfig config = { .green = 80, .yellow = 50, .type = FF_PERCENTAGE_TYPE_NUM_COLOR_BIT };

        VERIFY_NUM(0, config, false, "\e[31m0%\e[m");
        VERIFY_NUM(49, config, false, "\e[31m49%\e[m");
        VERIFY_NUM(50, config, false, "\e[33m50%\e[m");
        VERIFY_NUM(79, config, false, "\e[33m79%\e[m");
        VERIFY_NUM(80, config, false, "\e[32m80%\e[m");
        VERIFY_NUM(100, config, false, "\e[32m100%\e[m");
    }

    // Without the color flag the number is never colored, even when colors are enabled
    {
        options->pipe = false;
        const FFPercentageModuleConfig config = { .green = 50, .yellow = 80, .type = FF_PERCENTAGE_TYPE_NUM_BIT };

        VERIFY_NUM(0, config, false, "0%");
        VERIFY_NUM(100, config, false, "100%");
    }

    // A zero `type` means "use the global default" instead of "no flags"
    {
        options->pipe = false;
        FFPercentageModuleConfig config = { .green = 50, .yellow = 80, .type = 0 };

        options->percentType = FF_PERCENTAGE_TYPE_NUM_BIT;
        VERIFY_NUM(100, config, false, "100%");

        options->percentType = FF_PERCENTAGE_TYPE_NUM_BIT | FF_PERCENTAGE_TYPE_NUM_COLOR_BIT;
        VERIFY_NUM(100, config, false, "\e[31m100%\e[m");
    }

    // The bar fills `percent` of the width, rounding to the nearest block. Colors are left to the
    // `pipe` free cases above; the block characters and the borders are what is checked here.
    {
        options->pipe = true;
        const FFPercentageModuleConfig config = { .green = 50, .yellow = 80, .type = 0 };

        options->barWidth = 10;
        VERIFY_BAR(0, config, "[ ---------- ]");
        VERIFY_BAR(50, config, "[ ■■■■■----- ]");
        VERIFY_BAR(100, config, "[ ■■■■■■■■■■ ]");

        // An unset percentage fills the bar with the "total" character, same as zero
        VERIFY_BAR(-DBL_MAX, config, "[ ---------- ]");

        // The half block is rounded up at exactly 0.5
        VERIFY_BAR(4, config, "[ ---------- ]");
        VERIFY_BAR(5, config, "[ ■--------- ]");
        VERIFY_BAR(14, config, "[ ■--------- ]");
        VERIFY_BAR(15, config, "[ ■■-------- ]");

        options->barWidth = 4;
        VERIFY_BAR(0, config, "[ ---- ]");
        VERIFY_BAR(25, config, "[ ■--- ]");
        VERIFY_BAR(50, config, "[ ■■-- ]");
        VERIFY_BAR(100, config, "[ ■■■■ ]");

        // The borders are configurable and may be removed entirely
        ffStrbufSetS(&options->barBorderLeft, "");
        ffStrbufSetS(&options->barBorderRight, "");
        VERIFY_BAR(50, config, "■■--");

        ffStrbufSetS(&options->barBorderLeft, "<");
        ffStrbufSetS(&options->barBorderRight, ">");
        VERIFY_BAR(50, config, "<■■-->");

        ffStrbufSetS(&options->barBorderLeft, "[ ");
        ffStrbufSetS(&options->barBorderRight, " ]");

        // The bar characters themselves are configurable
        ffStrbufSetS(&options->barCharElapsed, "#");
        ffStrbufSetS(&options->barCharTotal, ".");
        VERIFY_BAR(50, config, "[ ##.. ]");
        ffStrbufSetS(&options->barCharElapsed, "■");
        ffStrbufSetS(&options->barCharTotal, "-");

        options->barWidth = 10;
    }

    ffOptionDestroyModuleArg(&module);
    ffOptionsDestroyDisplay(options);

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
