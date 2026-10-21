#include "common/endian.h"
#include "common/textModifier.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void verify(bool expression, const char* expressionStr, int lineNo) {
    if (expression) {
        return;
    }

    fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s\n" FASTFETCH_TEXT_MODIFIER_RESET, lineNo, expressionStr);
    exit(1);
}

#define VERIFY(expression) verify((expression), #expression, __LINE__)

// The macros take a value, so the only way to test them is to lay out the bytes of a known number
// in memory and read them back through the macro. The expectations below do not depend on the
// endianness of the machine running the test:
//
//   * copying the little endian byte sequence of 0x11223344 (44 33 22 11) into a `uint32_t` yields
//     0x11223344 on a little endian machine and 0x44332211 on a big endian one. `FF_READ_LE` has to
//     turn both of them into 0x11223344, which means it is the identity on one machine and a byte
//     swap on the other.
//   * `FF_READ_BE` is the mirror image.
//
// On a little endian machine this only pins that the two macros are the identity / swap pair; the
// selection itself (a compiler that does not define `__BIG_ENDIAN__` at all must still be detected
// as big endian from `__BYTE_ORDER__`) can only be exercised by a big endian build, or by a cross
// target such as `clang --target=s390x-linux-gnu`.
static void verify16(void) {
    const uint8_t littleEndianBytes[2] = { 0x22, 0x11 };
    const uint8_t bigEndianBytes[2] = { 0x11, 0x22 };

    uint16_t value;

    memcpy(&value, littleEndianBytes, sizeof(value));
    VERIFY(FF_READ_LE(value) == (uint16_t) 0x1122);

    memcpy(&value, bigEndianBytes, sizeof(value));
    VERIFY(FF_READ_BE(value) == (uint16_t) 0x1122);
}

static void verify32(void) {
    const uint8_t littleEndianBytes[4] = { 0x44, 0x33, 0x22, 0x11 };
    const uint8_t bigEndianBytes[4] = { 0x11, 0x22, 0x33, 0x44 };

    uint32_t value;

    memcpy(&value, littleEndianBytes, sizeof(value));
    VERIFY(FF_READ_LE(value) == 0x11223344u);

    memcpy(&value, bigEndianBytes, sizeof(value));
    VERIFY(FF_READ_BE(value) == 0x11223344u);
}

static void verify64(void) {
    const uint8_t littleEndianBytes[8] = { 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11 };
    const uint8_t bigEndianBytes[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };

    uint64_t value;

    memcpy(&value, littleEndianBytes, sizeof(value));
    VERIFY(FF_READ_LE(value) == 0x1122334455667788ull);

    memcpy(&value, bigEndianBytes, sizeof(value));
    VERIFY(FF_READ_BE(value) == 0x1122334455667788ull);
}

int main(void) {
    verify16();
    verify32();
    verify64();

    // A byte swap has to stay inside the width of the value it was given. A value whose bytes are
    // all equal reads the same through either macro, on either kind of machine.
    {
        uint16_t narrow = 0x0F0Fu;
        VERIFY(FF_READ_LE(narrow) == (uint16_t) 0x0F0F);
        VERIFY(FF_READ_BE(narrow) == (uint16_t) 0x0F0F);

        uint32_t wide = 0x0F0F0F0Fu;
        VERIFY(FF_READ_LE(wide) == 0x0F0F0F0Fu);
        VERIFY(FF_READ_BE(wide) == 0x0F0F0F0Fu);

        uint64_t widest = 0x0F0F0F0F0F0F0F0Full;
        VERIFY(FF_READ_LE(widest) == 0x0F0F0F0F0F0F0F0Full);
        VERIFY(FF_READ_BE(widest) == 0x0F0F0F0F0F0F0F0Full);
    }

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
