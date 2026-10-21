#include "common/option.h"
#include "common/temps.h"
#include "common/textModifier.h"
#include "fastfetch.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>

static FFModuleArgs module;

static void verify(double celsius, FFColorRangeConfig config, const char* expected, int lineNo) {
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    ffTempsAppendNum(celsius, &result, config, &module);

    if (!ffStrbufEqualS(&result, expected)) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffTempsAppendNum(%f, {%u, %u}): expected \"%s\", got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET,
            lineNo, celsius, config.green, config.yellow, expected, result.chars);
        exit(1);
    }
}

#define VERIFY_TEMP(celsius, config, expected) verify((celsius), (config), (expected), __LINE__)

int main(void) {
    // Use the real defaults rather than a hand built config, so a changed default is caught here too
    ffOptionsInitDisplay(&instance.config.display);
    FFOptionsDisplay* options = &instance.config.display;
    ffOptionInitModuleArg(&module, "");

    // Pin the threshold colors: the yellow and red defaults depend on whether the terminal is
    // detected as a light theme, which is not something a test should depend on.
    ffStrbufSetS(&options->tempColorGreen, "32");
    ffStrbufSetS(&options->tempColorYellow, "33");
    ffStrbufSetS(&options->tempColorRed, "31");

    const FFColorRangeConfig config = { .green = 50, .yellow = 80 };

    // An unknown temperature prints nothing at all, not even the unit
    {
        options->pipe = true;
        VERIFY_TEMP(-DBL_MAX, config, "");
    }

    // The default unit is Celsius, with one digit after the point
    {
        options->pipe = true;
        VERIFY_TEMP(0, config, "0.0°C");
        VERIFY_TEMP(25.5, config, "25.5°C");
        VERIFY_TEMP(-10, config, "-10.0°C");
        VERIFY_TEMP(-273.15, config, "-273.1°C");
    }

    // tempNdigits
    {
        options->pipe = true;
        options->tempNdigits = 0;
        VERIFY_TEMP(25.4, config, "25°C");
        VERIFY_TEMP(25.6, config, "26°C");
        VERIFY_TEMP(0, config, "0°C");

        options->tempNdigits = 2;
        VERIFY_TEMP(25.5, config, "25.50°C");

        options->tempNdigits = 1;
    }

    // Celsius and Fahrenheit only take a space before the unit when it is forced on, while Kelvin
    // takes one by default. That asymmetry is deliberate and easy to break.
    {
        options->pipe = true;
        options->tempNdigits = 2; // so that 25.5 + 273.15 prints exactly

        options->tempSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_DEFAULT;
        VERIFY_TEMP(25.5, config, "25.50°C");
        options->tempUnit = FF_TEMPERATURE_UNIT_KELVIN;
        VERIFY_TEMP(25.5, config, "298.65 K");
        options->tempUnit = FF_TEMPERATURE_UNIT_DEFAULT;

        options->tempSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_NEVER;
        VERIFY_TEMP(25.5, config, "25.50°C");
        options->tempUnit = FF_TEMPERATURE_UNIT_KELVIN;
        VERIFY_TEMP(25.5, config, "298.65K");
        options->tempUnit = FF_TEMPERATURE_UNIT_DEFAULT;

        options->tempSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_ALWAYS;
        VERIFY_TEMP(25.5, config, "25.50 °C");
        options->tempUnit = FF_TEMPERATURE_UNIT_KELVIN;
        VERIFY_TEMP(25.5, config, "298.65 K");
        options->tempUnit = FF_TEMPERATURE_UNIT_DEFAULT;

        options->tempSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_DEFAULT;
        options->tempNdigits = 1;
    }

    // Fahrenheit and Kelvin are conversions of the Celsius value that the thresholds were compared
    // against, so the unit changes what is printed but never which band is picked.
    {
        options->pipe = true;

        options->tempUnit = FF_TEMPERATURE_UNIT_FAHRENHEIT;
        VERIFY_TEMP(0, config, "32.0°F");
        VERIFY_TEMP(100, config, "212.0°F");
        VERIFY_TEMP(37, config, "98.6°F");
        VERIFY_TEMP(-40, config, "-40.0°F"); // the one point where the two scales meet

        options->tempNdigits = 2;
        options->tempUnit = FF_TEMPERATURE_UNIT_KELVIN;
        VERIFY_TEMP(0, config, "273.15 K");
        VERIFY_TEMP(100, config, "373.15 K");
        VERIFY_TEMP(-273.15, config, "0.00 K");
        options->tempNdigits = 1;

        options->tempUnit = FF_TEMPERATURE_UNIT_DEFAULT;
    }

    // Colors, with the same band rules as the percentage module: the threshold itself belongs to the
    // lower band, and the reset sequence is always emitted when colors are on.
    {
        options->pipe = false;

        VERIFY_TEMP(0, config, "\e[32m0.0°C\e[m");
        VERIFY_TEMP(50, config, "\e[32m50.0°C\e[m");
        VERIFY_TEMP(51, config, "\e[33m51.0°C\e[m");
        VERIFY_TEMP(80, config, "\e[33m80.0°C\e[m");
        VERIFY_TEMP(81, config, "\e[31m81.0°C\e[m");
        VERIFY_TEMP(100, config, "\e[31m100.0°C\e[m");

        // The color is chosen from the Celsius value, so a display unit that is not Celsius does not
        // move the threshold
        options->tempUnit = FF_TEMPERATURE_UNIT_FAHRENHEIT;
        VERIFY_TEMP(81, config, "\e[31m177.8°F\e[m");
        options->tempUnit = FF_TEMPERATURE_UNIT_DEFAULT;

        // An unknown temperature has no color at all
        VERIFY_TEMP(-DBL_MAX, config, "");

        // When `green > yellow` the bands are inverted, so a low value is the alarming one
        const FFColorRangeConfig inverted = { .green = 80, .yellow = 50 };
        VERIFY_TEMP(0, inverted, "\e[31m0.0°C\e[m");
        VERIFY_TEMP(49, inverted, "\e[31m49.0°C\e[m");
        VERIFY_TEMP(50, inverted, "\e[33m50.0°C\e[m");
        VERIFY_TEMP(79, inverted, "\e[33m79.0°C\e[m");
        VERIFY_TEMP(80, inverted, "\e[32m80.0°C\e[m");
        VERIFY_TEMP(100, inverted, "\e[32m100.0°C\e[m");
    }

    // The color is restored to the module's output color, so the rest of the line keeps its own
    {
        options->pipe = false;
        ffStrbufSetS(&module.outputColor, "94");
        VERIFY_TEMP(0, config, "\e[32m0.0°C\e[m\e[94m");
        ffStrbufClear(&module.outputColor);

        // ... and it falls back to the global output color when the module has none
        ffStrbufSetS(&options->colorOutput, "95");
        VERIFY_TEMP(0, config, "\e[32m0.0°C\e[m\e[95m");
        ffStrbufClear(&options->colorOutput);
    }

    // The result is appended to the buffer instead of replacing it
    {
        options->pipe = true;
        FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateS("Temp: ");
        ffTempsAppendNum(25.5, &result, config, &module);
        if (!ffStrbufEqualS(&result, "Temp: 25.5°C")) {
            fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffTempsAppendNum did not append: got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET, __LINE__, result.chars);
            exit(1);
        }
    }

    ffOptionDestroyModuleArg(&module);
    ffOptionsDestroyDisplay(options);

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
