#include "common/frequency.h"
#include "common/textModifier.h"
#include "fastfetch.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void verify(uint32_t mhz, bool expectedResult, const char* expected, int lineNo) {
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    const bool returned = ffFreqAppendNum(mhz, &result);

    if (returned != expectedResult || !ffStrbufEqualS(&result, expected)) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffFreqAppendNum(%u): expected %s \"%s\", got %s \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET,
            lineNo, mhz, expectedResult ? "true" : "false", expected, returned ? "true" : "false", result.chars);
        exit(1);
    }
}

#define VERIFY_FREQ(mhz, expectedResult, expected) verify((mhz), (expectedResult), (expected), __LINE__)

int main(void) {
    // Use the real defaults rather than a hand built config, so a changed default is caught here too
    ffOptionsInitDisplay(&instance.config.display);
    FFOptionsDisplay* options = &instance.config.display;

    // An unknown frequency prints nothing at all
    {
        VERIFY_FREQ(0, false, "");
    }

    // With `freqNdigits >= 0` the value is printed in GHz, with that many digits after the point
    {
        VERIFY_FREQ(1000, true, "1.00 GHz");
        VERIFY_FREQ(3600, true, "3.60 GHz");
        VERIFY_FREQ(5000, true, "5.00 GHz");
        VERIFY_FREQ(100, true, "0.10 GHz");
        VERIFY_FREQ(1, true, "0.00 GHz"); // rounds down to zero, but is still reported
        VERIFY_FREQ(4294000, true, "4294.00 GHz"); // the whole `uint32_t` range is usable
    }

    // `freqNdigits = 0` rounds to whole GHz
    {
        options->freqNdigits = 0;
        VERIFY_FREQ(1000, true, "1 GHz");
        VERIFY_FREQ(3600, true, "4 GHz"); // rounded, not truncated
        VERIFY_FREQ(1400, true, "1 GHz");
        VERIFY_FREQ(0, false, "");
        options->freqNdigits = 2;
    }

    // A negative `freqNdigits` selects the integer MHz form instead
    {
        options->freqNdigits = -1;
        VERIFY_FREQ(3600, true, "3600 MHz");
        VERIFY_FREQ(1000, true, "1000 MHz");
        VERIFY_FREQ(1, true, "1 MHz");
        VERIFY_FREQ(0, false, "");
        options->freqNdigits = 2;
    }

    // Only `never` removes the space between the number and the unit
    {
        options->freqSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_NEVER;
        VERIFY_FREQ(3600, true, "3.60GHz");
        options->freqNdigits = -1;
        VERIFY_FREQ(3600, true, "3600MHz");
        options->freqNdigits = 2;

        options->freqSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_ALWAYS;
        VERIFY_FREQ(3600, true, "3.60 GHz");

        options->freqSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_DEFAULT;
        VERIFY_FREQ(3600, true, "3.60 GHz");
    }

    // The result is appended to the buffer instead of replacing it
    {
        FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateS("CPU: ");
        if (!ffFreqAppendNum(3600, &result) || !ffStrbufEqualS(&result, "CPU: 3.60 GHz")) {
            fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffFreqAppendNum did not append: got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET, __LINE__, result.chars);
            exit(1);
        }
    }

    ffOptionsDestroyDisplay(options);

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
