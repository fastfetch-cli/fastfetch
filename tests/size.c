#include "common/size.h"
#include "common/textModifier.h"
#include "fastfetch.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void verify(uint64_t bytes, const char* expected, int lineNo) {
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    ffSizeAppendNum(bytes, &result);

    if (!ffStrbufEqualS(&result, expected)) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffSizeAppendNum(%llu): expected \"%s\", got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET,
            lineNo, (unsigned long long) bytes, expected, result.chars);
        exit(1);
    }
}

#define VERIFY_SIZE(bytes, expected) verify((bytes), (expected), __LINE__)

int main(void) {
    // Use the real defaults rather than a hand built config, so a changed default is caught here too
    ffOptionsInitDisplay(&instance.config.display);
    FFOptionsDisplay* options = &instance.config.display;

    // IEC (the default): 1024 based, with the `KiB` style suffixes
    {
        VERIFY_SIZE(0, "0 B");
        VERIFY_SIZE(1, "1 B");
        VERIFY_SIZE(999, "999 B");
        VERIFY_SIZE(1023, "1023 B"); // one byte short of the first prefix
        VERIFY_SIZE(1024, "1.00 KiB");
        VERIFY_SIZE(1025, "1.00 KiB"); // below the rounding precision
        VERIFY_SIZE(1536, "1.50 KiB");
        VERIFY_SIZE(1024ULL * 1024, "1.00 MiB");
        VERIFY_SIZE(1024ULL * 1024 * 1024, "1.00 GiB");
        VERIFY_SIZE(1024ULL * 1024 * 1024 * 1024, "1.00 TiB");
        VERIFY_SIZE(1024ULL * 1024 * 1024 * 1024 * 1024, "1.00 PiB");
        VERIFY_SIZE(1024ULL * 1024 * 1024 * 1024 * 1024 * 1024, "1.00 EiB");

        // The largest 64 bit value is 16 EiB; `ZiB` and `YiB` are listed but can never be reached
        // through an IEC conversion of a `uint64_t`.
        VERIFY_SIZE(UINT64_MAX, "16.00 EiB");
    }

    // SI: 1000 based, with the `kB` style suffixes
    {
        options->sizeBinaryPrefix = FF_SIZE_BINARY_PREFIX_TYPE_SI;

        VERIFY_SIZE(0, "0 B");
        VERIFY_SIZE(999, "999 B");
        VERIFY_SIZE(1000, "1.00 kB");
        VERIFY_SIZE(1500, "1.50 kB");
        VERIFY_SIZE(1000000, "1.00 MB");
        VERIFY_SIZE(1000000000ULL, "1.00 GB");
        VERIFY_SIZE(1000000000000ULL, "1.00 TB");
        VERIFY_SIZE(1000000000000000ULL, "1.00 PB");
        VERIFY_SIZE(1000000000000000000ULL, "1.00 EB");
        VERIFY_SIZE(UINT64_MAX, "18.45 EB");
    }

    // JEDEC: 1024 based, but with the `KB` style suffixes. The suffix list stops at `TB`, so the
    // loop has to stop there as well instead of reading past the end.
    {
        options->sizeBinaryPrefix = FF_SIZE_BINARY_PREFIX_TYPE_JEDEC;

        VERIFY_SIZE(1023, "1023 B");
        VERIFY_SIZE(1024, "1.00 KB");
        VERIFY_SIZE(1024ULL * 1024, "1.00 MB");
        VERIFY_SIZE(1024ULL * 1024 * 1024, "1.00 GB");
        VERIFY_SIZE(1024ULL * 1024 * 1024 * 1024, "1.00 TB");
        VERIFY_SIZE(1024ULL * 1024 * 1024 * 1024 * 1024, "1024.00 TB"); // no `PB` in JEDEC
        VERIFY_SIZE(UINT64_MAX, "16777216.00 TB");
    }

    options->sizeBinaryPrefix = FF_SIZE_BINARY_PREFIX_TYPE_IEC;

    // sizeNdigits: the number of digits after the decimal point. It applies only once a prefix is
    // used -- a plain byte count is always printed as an integer.
    {
        options->sizeNdigits = 0;
        VERIFY_SIZE(1024, "1 KiB");
        VERIFY_SIZE(1536, "2 KiB"); // rounded, not truncated
        VERIFY_SIZE(512, "512 B");
        VERIFY_SIZE(1023, "1023 B");

        options->sizeNdigits = 4;
        VERIFY_SIZE(1024, "1.0000 KiB");
        VERIFY_SIZE(1536, "1.5000 KiB");
        VERIFY_SIZE(512, "512 B"); // still an integer
        VERIFY_SIZE(0, "0 B");

        options->sizeNdigits = 2;
    }

    // sizeMaxPrefix: the index of the largest suffix that may be used. `0` disables the conversion
    // entirely.
    {
        options->sizeMaxPrefix = 0;
        VERIFY_SIZE(0, "0 B");
        VERIFY_SIZE(1024, "1024 B");
        VERIFY_SIZE(UINT64_MAX, "18446744073709551615 B");

        options->sizeMaxPrefix = 1;
        VERIFY_SIZE(1024, "1.00 KiB");
        VERIFY_SIZE(1024ULL * 1024, "1024.00 KiB");
        VERIFY_SIZE(1024ULL * 1024 * 1024, "1048576.00 KiB");

        options->sizeMaxPrefix = 8;
    }

    // sizeSpaceBeforeUnit: only `never` removes the space, `default` and `always` both keep it
    {
        options->sizeSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_NEVER;
        VERIFY_SIZE(0, "0B");
        VERIFY_SIZE(1024, "1.00KiB");
        VERIFY_SIZE(1536, "1.50KiB");

        options->sizeSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_ALWAYS;
        VERIFY_SIZE(0, "0 B");
        VERIFY_SIZE(1024, "1.00 KiB");

        options->sizeSpaceBeforeUnit = FF_SPACE_BEFORE_UNIT_DEFAULT;
        VERIFY_SIZE(1024, "1.00 KiB");
    }

    // The result is appended to the buffer instead of replacing it
    {
        FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateS("Size: ");
        ffSizeAppendNum(1024, &result);
        if (!ffStrbufEqualS(&result, "Size: 1.00 KiB")) {
            fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffSizeAppendNum did not append: got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET, __LINE__, result.chars);
            exit(1);
        }
    }

    ffOptionsDestroyDisplay(options);

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
