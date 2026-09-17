// This translation unit pretends to be compiled on a big endian host, so that the endianness
// detection in `common/endian.h` can be exercised on a little endian machine.
//
// The selection is a preprocessor decision, so it can be pinned with `_Static_assert` instead of by
// running anything -- which matters, because the alternative would be to observe a byte swap at run
// time on hardware that really is little endian, and that cannot work.
//
// The case this guards: GCC does not define `__BIG_ENDIAN__` at all, it only documents
// `__BYTE_ORDER__` and the `__ORDER_*_ENDIAN__` values. A header that tests `#if __BIG_ENDIAN__`
// therefore evaluates it as 0 on s390x and silently picks the little endian branch, byte swapping
// values that were already big endian. `__BIG_ENDIAN__` is undefined here for exactly that reason:
// a header that only looks at its presence, or at its value, fails the assertions below.
//
// System headers are pulled in before the macros are faked so that they are not affected.

#include <stdint.h>
#include <stdio.h>

#include "common/textModifier.h"

// What a big endian compiler reports ...
#undef __BYTE_ORDER__
#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
// ... and what GCC does not report on such a machine, even though it is one.
#undef __BIG_ENDIAN__

#include "common/endian.h"

// On a big endian host the byte order of the machine and the byte order of the data agree, so
// reading a big endian value is a no-op ...
_Static_assert(FF_READ_BE((uint16_t) 0x1122) == (uint16_t) 0x1122, "FF_READ_BE must be the identity on a big endian host");
_Static_assert(FF_READ_BE(0x11223344u) == 0x11223344u, "FF_READ_BE must be the identity on a big endian host");
_Static_assert(FF_READ_BE(0x1122334455667788ull) == 0x1122334455667788ull, "FF_READ_BE must be the identity on a big endian host");

// ... and reading a little endian one swaps.
_Static_assert(FF_READ_LE((uint16_t) 0x1122) == (uint16_t) 0x2211, "FF_READ_LE must swap on a big endian host");
_Static_assert(FF_READ_LE(0x11223344u) == 0x44332211u, "FF_READ_LE must swap on a big endian host");
_Static_assert(FF_READ_LE(0x1122334455667788ull) == 0x8877665544332211ull, "FF_READ_LE must swap on a big endian host");

int main(void) {
    // Everything this test asserts is checked at compile time, so reaching this line is the result.
    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
