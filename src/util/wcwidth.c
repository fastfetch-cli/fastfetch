#include "wcwidth.h"
#include "3rdparty/widecharwidth/widechar_width_c.h"

int mk_wcwidth(uint32_t wc)
{
    // // We render U+1F6E1 (🛡) with a width of 2,
    // // but widechar_width says it has a width of 1 because Unicode classifies it as "neutral".
    // //
    // // So we simply decide the width ourselves
    // if (wc == 0x1F6E1) return 2;
    //
    // Well terminals do show it as width 1 after all

    int width = widechar_wcwidth(wc);

    switch (width) {
        case widechar_ambiguous:
        case widechar_private_use:
            return 1;
        case widechar_widened_in_9:
            // Our renderer supports Unicode 9
            return 2;
        // case widechar_nonprint:
        // case widechar_combining:
        // case widechar_unassigned:
        // case widechar_non_character:
        //    return -1;
        default:
            // Use the width widechar_width gave us.
            return width;
    }
}
