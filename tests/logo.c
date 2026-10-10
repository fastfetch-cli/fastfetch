#include "logo/logo.h"
#include "common/textModifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

noreturn static void testFailed(const char* expression, int lineNo) {
    fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s\n" FASTFETCH_TEXT_MODIFIER_RESET, lineNo, expression);
    exit(1);
}

noreturn static void testFailedEnum(FFLogoPosition expected, FFLogoPosition actual, const char* what, int lineNo) {
    fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s: expected %u, got %u\n" FASTFETCH_TEXT_MODIFIER_RESET, lineNo, what, (unsigned) expected, (unsigned) actual);
    exit(1);
}

#define VERIFY(expression) \
    if (!(expression)) testFailed(#expression, __LINE__)

#define VERIFY_POSITION(expected, actual, what) \
    if ((expected) != (actual)) testFailedEnum((expected), (actual), (what), __LINE__)

int main(void) {
    // Unknown terminal size keeps the historical left-side layout.
    VERIFY_POSITION(FF_LOGO_POSITION_LEFT,
        ffLogoSelectPosition(FF_LOGO_POSITION_AUTO, 0, 40),
        "auto fallback");

    // Auto keeps the logo beside the output while at least 32 columns remain for module text.
    VERIFY_POSITION(FF_LOGO_POSITION_LEFT,
        ffLogoSelectPosition(FF_LOGO_POSITION_AUTO, 72, 40),
        "auto exact side-by-side fit");
    VERIFY_POSITION(FF_LOGO_POSITION_TOP,
        ffLogoSelectPosition(FF_LOGO_POSITION_AUTO, 71, 40),
        "auto narrow terminal");
    VERIFY_POSITION(FF_LOGO_POSITION_TOP,
        ffLogoSelectPosition(FF_LOGO_POSITION_AUTO, 80, UINT32_MAX),
        "auto width overflow");

    // Explicit positions are never overridden.
    VERIFY_POSITION(FF_LOGO_POSITION_LEFT,
        ffLogoSelectPosition(FF_LOGO_POSITION_LEFT, 20, 40),
        "explicit left");
    VERIFY_POSITION(FF_LOGO_POSITION_TOP,
        ffLogoSelectPosition(FF_LOGO_POSITION_TOP, 120, 40),
        "explicit top");
    VERIFY_POSITION(FF_LOGO_POSITION_RIGHT,
        ffLogoSelectPosition(FF_LOGO_POSITION_RIGHT, 20, 40),
        "explicit right");

    VERIFY(strcmp(ffLogoPositionToString(FF_LOGO_POSITION_AUTO), "auto") == 0);
    VERIFY(strcmp(ffLogoPositionToString(FF_LOGO_POSITION_LEFT), "left") == 0);
    VERIFY(strcmp(ffLogoPositionToString(FF_LOGO_POSITION_TOP), "top") == 0);
    VERIFY(strcmp(ffLogoPositionToString(FF_LOGO_POSITION_RIGHT), "right") == 0);

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
