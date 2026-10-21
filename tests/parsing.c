#include "common/parsing.h"
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

static void verifyVersionCompare(uint32_t major1, uint32_t minor1, uint32_t patch1, uint32_t major2, uint32_t minor2, uint32_t patch2, int8_t expected, int lineNo) {
    const FFVersion first = { major1, minor1, patch1 };
    const FFVersion second = { major2, minor2, patch2 };

    const int8_t forward = ffVersionCompare(&first, &second);
    const int8_t backward = ffVersionCompare(&second, &first);

    // Only the sign is meaningful to callers, but the comparison is documented to be antisymmetric,
    // so a caller that negates the result to get "greater" must not be surprised.
    if (forward != expected || backward != -expected) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffVersionCompare(%u.%u.%u, %u.%u.%u): expected %d/%d, got %d/%d\n" FASTFETCH_TEXT_MODIFIER_RESET,
            lineNo, major1, minor1, patch1, major2, minor2, patch2, expected, -expected, forward, backward);
        exit(1);
    }
}

#define VERIFY_COMPARE(...) verifyVersionCompare(__VA_ARGS__, __LINE__)

static void verifyPretty(uint32_t major, uint32_t minor, uint32_t patch, const char* expected, int lineNo) {
    const FFVersion version = { major, minor, patch };
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    ffVersionToPretty(&version, &result);

    if (!ffStrbufEqualS(&result, expected)) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffVersionToPretty(%u.%u.%u): expected \"%s\", got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET,
            lineNo, major, minor, patch, expected, result.chars);
        exit(1);
    }
}

#define VERIFY_PRETTY(...) verifyPretty(__VA_ARGS__, __LINE__)

static void verifySemver(const char* major, const char* minor, const char* patch, const char* expected, int lineNo) {
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY majorBuf = ffStrbufCreateS(major);
    FF_STRBUF_AUTO_DESTROY minorBuf = ffStrbufCreateS(minor);
    FF_STRBUF_AUTO_DESTROY patchBuf = ffStrbufCreateS(patch);

    ffParseSemver(&result, &majorBuf, &minorBuf, &patchBuf);

    if (!ffStrbufEqualS(&result, expected)) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffParseSemver(\"%s\", \"%s\", \"%s\"): expected \"%s\", got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET,
            lineNo, major, minor, patch, expected, result.chars);
        exit(1);
    }
}

#define VERIFY_SEMVER(...) verifySemver(__VA_ARGS__, __LINE__)

static void verifyGtk(const char* gtk2, const char* gtk3, const char* gtk4, const char* expected, int lineNo) {
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY gtk2Buf = ffStrbufCreateS(gtk2);
    FF_STRBUF_AUTO_DESTROY gtk3Buf = ffStrbufCreateS(gtk3);
    FF_STRBUF_AUTO_DESTROY gtk4Buf = ffStrbufCreateS(gtk4);

    ffParseGTK(&result, &gtk2Buf, &gtk3Buf, &gtk4Buf);

    if (!ffStrbufEqualS(&result, expected)) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffParseGTK(\"%s\", \"%s\", \"%s\"): expected \"%s\", got \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET,
            lineNo, gtk2, gtk3, gtk4, expected, result.chars);
        exit(1);
    }
}

#define VERIFY_GTK(...) verifyGtk(__VA_ARGS__, __LINE__)

int main(void) {
    // ffVersionCompare: the major field decides first, then minor, then patch. Equal versions
    // compare equal even when they come from different sources.
    {
        VERIFY_COMPARE(0, 0, 0, 0, 0, 0, 0);
        VERIFY_COMPARE(1, 2, 3, 1, 2, 3, 0);
        VERIFY_COMPARE(UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, 0);

        VERIFY_COMPARE(2, 0, 0, 1, 99, 99, 1);
        VERIFY_COMPARE(1, 99, 99, 2, 0, 0, -1);

        VERIFY_COMPARE(1, 2, 0, 1, 1, 99, 1);
        VERIFY_COMPARE(1, 1, 99, 1, 2, 0, -1);

        VERIFY_COMPARE(1, 2, 3, 1, 2, 2, 1);
        VERIFY_COMPARE(1, 2, 2, 1, 2, 3, -1);

        // A larger patch must not be able to outvote a smaller minor
        VERIFY_COMPARE(1, 2, 0, 1, 1, UINT32_MAX, 1);
        VERIFY_COMPARE(1, 1, UINT32_MAX, 1, 2, 0, -1);

        // ... nor a larger minor a smaller major
        VERIFY_COMPARE(2, 0, 0, 1, UINT32_MAX, UINT32_MAX, 1);
        VERIFY_COMPARE(1, UINT32_MAX, UINT32_MAX, 2, 0, 0, -1);
    }

    // ffVersionToPretty: a zero version prints as nothing at all, and a field is only printed when
    // it or a later field is non-zero.
    {
        VERIFY_PRETTY(0, 0, 0, "");
        VERIFY_PRETTY(1, 0, 0, "1");
        VERIFY_PRETTY(0, 1, 0, "0.1");
        VERIFY_PRETTY(0, 0, 1, "0.0.1");
        VERIFY_PRETTY(1, 2, 0, "1.2");
        VERIFY_PRETTY(1, 0, 3, "1.0.3");
        VERIFY_PRETTY(1, 2, 3, "1.2.3");
        VERIFY_PRETTY(10, 20, 30, "10.20.30");
        VERIFY_PRETTY(UINT32_MAX, 0, 0, "4294967295");
        VERIFY_PRETTY(0, 0, UINT32_MAX, "0.0.4294967295");
    }

    // ffParseSemver: joins whichever parts were detected. A missing major is reported as 1, which is
    // how "2.3" from a source that only reports the last two components stays unambiguous.
    {
        VERIFY_SEMVER("1", "2", "3", "1.2.3");
        VERIFY_SEMVER("1", "2", "", "1.2");
        VERIFY_SEMVER("1", "", "", "1");
        VERIFY_SEMVER("1", "", "3", "1.0.3");
        VERIFY_SEMVER("", "2", "3", "1.2.3");
        VERIFY_SEMVER("", "2", "", "1.2");
        VERIFY_SEMVER("", "", "3", "1.0.3");
        VERIFY_SEMVER("", "", "", "");

        // A zero part is a value, not a missing part
        VERIFY_SEMVER("0", "0", "0", "0.0.0");
        VERIFY_SEMVER("0", "", "", "0");
    }

    // ffParseSemver appends instead of replacing, so a caller can prefix its own text
    {
        FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateS("v");
        FF_STRBUF_AUTO_DESTROY majorBuf = ffStrbufCreateS("1");
        FF_STRBUF_AUTO_DESTROY minorBuf = ffStrbufCreateS("2");
        FF_STRBUF_AUTO_DESTROY patchBuf = ffStrbufCreateS("3");
        ffParseSemver(&result, &majorBuf, &minorBuf, &patchBuf);
        VERIFY(ffStrbufEqualS(&result, "v1.2.3"));
    }

    // ffParseGTK: the three versions are folded into one line, and every combination of present and
    // equal versions has its own suffix.
    {
        // All three present
        VERIFY_GTK("1.0", "1.0", "1.0", "1.0 [GTK2/3/4]");
        VERIFY_GTK("2.0", "2.0", "3.0", "2.0 [GTK2/3], 3.0 [GTK4]");
        VERIFY_GTK("2.0", "3.0", "3.0", "2.0 [GTK2], 3.0 [GTK3/4]");
        VERIFY_GTK("2.0", "3.0", "4.0", "2.0 [GTK2], 3.0 [GTK3], 4.0 [GTK4]");

        // GTK2 and GTK4 equal while GTK3 differs is not special cased: it falls back to the plain
        // three-way listing, so the same version is printed twice with different suffixes.
        VERIFY_GTK("2.0", "3.0", "2.0", "2.0 [GTK2], 3.0 [GTK3], 2.0 [GTK4]");

        // The comparison is case insensitive, but the value printed is the one from the buffer that
        // the branch selected -- here the third one, not the first.
        VERIFY_GTK("ABC", "abc", "aBc", "aBc [GTK2/3/4]");
        VERIFY_GTK("2.0", "2.0", "3.0", "2.0 [GTK2/3], 3.0 [GTK4]");

        // Two present
        VERIFY_GTK("1.0", "1.0", "", "1.0 [GTK2/3]");
        VERIFY_GTK("2.0", "3.0", "", "2.0 [GTK2], 3.0 [GTK3]");
        VERIFY_GTK("1.0", "", "1.0", "1.0 [GTK2/4]");
        VERIFY_GTK("2.0", "", "4.0", "2.0 [GTK2], 4.0 [GTK4]");
        VERIFY_GTK("", "1.0", "1.0", "1.0 [GTK3/4]");
        VERIFY_GTK("", "3.0", "4.0", "3.0 [GTK3], 4.0 [GTK4]");

        // One present
        VERIFY_GTK("2.0", "", "", "2.0 [GTK2]");
        VERIFY_GTK("", "3.0", "", "3.0 [GTK3]");
        VERIFY_GTK("", "", "4.0", "4.0 [GTK4]");

        // None present
        VERIFY_GTK("", "", "", "");
    }

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
