#include "common/duration.h"
#include "util/textModifier.h"
#include "fastfetch.h"

#include <stdlib.h>

static void verify(uint64_t totalSeconds, const char* expected, int lineNo)
{
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    ffDurationAppendNum(totalSeconds, &result);
    if (!ffStrbufEqualS(&result, expected))
    {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %llu: expected \"%s\", got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET, lineNo, (unsigned long long) totalSeconds, expected, result.chars);
        exit(1);
    }
}

#define VERIFY(color, expected) verify((color), (expected), __LINE__)

int main(void)
{
    // Test seconds less than 60
    VERIFY(0, "0 seconds");
    VERIFY(1, "1 second");
    VERIFY(2, "2 seconds");
    VERIFY(59, "59 seconds");

    // Test minute rounding (when seconds >= 30)
    VERIFY(60, "1 min");
    VERIFY(60 + 29, "1 min");
    VERIFY(60 + 30, "2 mins");

    // Test only minutes
    VERIFY(60 * 2 - 1, "2 mins");
    VERIFY(60 * 2, "2 mins");
    VERIFY(60 * 59 + 29, "59 mins");

    // Test only hours (no minutes)
    VERIFY(60 * 59 + 30, "1 hour");
    VERIFY(60 * 60, "1 hour");
    VERIFY(2 * 60 * 60, "2 hours");
    VERIFY(23 * 60 * 60, "23 hours");

    // Test combination of hours and minutes
    VERIFY(60 * 60 + 60, "1 hour, 1 min");
    VERIFY(60 * 60 + 60 * 2, "1 hour, 2 mins");
    VERIFY(60 * 60 * 2 + 60 + 29, "2 hours, 1 min");
    VERIFY(60 * 60 * 2 + 60 + 30, "2 hours, 2 mins");

    // Test days
    VERIFY(60 * 60 * 24, "1 day");
    VERIFY(60 * 60 * 24 - 1, "1 day");
    VERIFY(60 * 60 * 24 * 2, "2 days");

    // Test combination of days and hours
    VERIFY(60 * 60 * 24 + 60 * 60, "1 day, 1 hour");
    VERIFY(60 * 60 * 24 * 2 + 60 * 60, "2 days, 1 hour");
    VERIFY(60 * 60 * 24 * 2 + 60 * 60 * 2, "2 days, 2 hours");

    // Test combination of days, hours, and minutes
    VERIFY(60 * 60 * 24 + 60 * 60 + 60, "1 day, 1 hour, 1 min");
    VERIFY(60 * 60 * 24 * 2 + 60 * 60 + 60 * 2, "2 days, 1 hour, 2 mins");
    VERIFY(60 * 60 * 24 * 2 + 60 * 2, "2 days, 2 mins");

    // Test very large number of days
    VERIFY(60 * 60 * 24 * 100, "100 days(!)");
    VERIFY(60 * 60 * 24 * 200, "200 days(!)");

    instance.config.display.durationAbbreviation = true;
    instance.config.display.durationSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_NEVER;
    // Test seconds less than 60
    VERIFY(0, "0secs");
    VERIFY(1, "1sec");
    VERIFY(2, "2secs");
    VERIFY(59, "59secs");

    // Test minute rounding (when seconds >= 30)
    VERIFY(60, "1m");
    VERIFY(60 + 29, "1m");
    VERIFY(60 + 30, "2m");

    // Test only minutes
    VERIFY(60 * 2 - 1, "2m");
    VERIFY(60 * 2, "2m");
    VERIFY(60 * 59 + 29, "59m");

    // Test only hours (no minutes)
    VERIFY(60 * 59 + 30, "1h");
    VERIFY(60 * 60, "1h");
    VERIFY(2 * 60 * 60, "2h");
    VERIFY(23 * 60 * 60, "23h");

    // Test combination of hours and minutes
    VERIFY(60 * 60 + 60, "1h 1m");
    VERIFY(60 * 60 + 60 * 2, "1h 2m");
    VERIFY(60 * 60 * 2 + 60 + 29, "2h 1m");
    VERIFY(60 * 60 * 2 + 60 + 30, "2h 2m");

    // Test days
    VERIFY(60 * 60 * 24, "1d");
    VERIFY(60 * 60 * 24 - 1, "1d");
    VERIFY(60 * 60 * 24 * 2, "2d");

    // Test combination of days and hours
    VERIFY(60 * 60 * 24 + 60 * 60, "1d 1h");
    VERIFY(60 * 60 * 24 * 2 + 60 * 60, "2d 1h");
    VERIFY(60 * 60 * 24 * 2 + 60 * 60 * 2, "2d 2h");

    // Test combination of days, hours, and minutes
    VERIFY(60 * 60 * 24 + 60 * 60 + 60, "1d 1h 1m");
    VERIFY(60 * 60 * 24 * 2 + 60 * 60 + 60 * 2, "2d 1h 2m");
    VERIFY(60 * 60 * 24 * 2 + 60 * 2, "2d 2m");

    // Test very large number of days
    VERIFY(60 * 60 * 24 * 100, "100d");
    VERIFY(60 * 60 * 24 * 200, "200d");

    //Success
    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
}
