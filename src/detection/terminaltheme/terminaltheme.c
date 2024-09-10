#include "terminaltheme.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

#include <inttypes.h>

static bool detectByEscapeCode(FFTerminalThemeResult* result)
{
    // Windows Terminal removes all `\e`s in its output
    if (ffGetTerminalResponse("\e]10;?\e\\" /*fg*/ "\e]11;?\e\\" /*bg*/,
        6,
        "%*[^0-9]10;rgb:%" SCNx16 "/%" SCNx16 "/%" SCNx16 /*"\e\\"*/ "%*[^0-9]11;rgb:%" SCNx16 "/%" SCNx16 "/%" SCNx16 /*"\e\\"*/,
        &result->fg.r, &result->fg.g, &result->fg.b,
        &result->bg.r, &result->bg.g, &result->bg.b) == NULL)
    {
        if (result->fg.r > 0x0100 || result->fg.g > 0x0100 || result->fg.b > 0x0100)
            result->fg.r /= 0x0100, result->fg.g /= 0x0100, result->fg.b /= 0x0100;
        if (result->bg.r > 0x0100 || result->bg.g > 0x0100 || result->bg.b > 0x0100)
            result->bg.r /= 0x0100, result->bg.g /= 0x0100, result->bg.b /= 0x0100;
    }
    else
        return false;

    return true;
}

static FFTerminalThemeColor fgbgToColor(int num)
{
    // https://github.com/dalance/termbg/blob/13c478a433fa182e65c401d26a1e7792a7f7f453/src/lib.rs#L251
    switch (num)
    {
        case  0: return (FFTerminalThemeColor){  0,   0,   0, false}; // black
        case  1: return (FFTerminalThemeColor){205,   0,   0, false}; // red
        case  2: return (FFTerminalThemeColor){  0, 205,   0, false}; // green
        case  3: return (FFTerminalThemeColor){205, 205,   0, false}; // yellow
        case  4: return (FFTerminalThemeColor){  0,   0, 238, false}; // blue
        case  5: return (FFTerminalThemeColor){205,   0, 205, false}; // magenta
        case  6: return (FFTerminalThemeColor){  0, 205, 205, false}; // cyan
        case  7: return (FFTerminalThemeColor){229, 229, 229, false}; // white

        case  8: return (FFTerminalThemeColor){127, 127, 127, false}; // bright black
        case  9: return (FFTerminalThemeColor){255,   0,   0, false}; // bright red
        case 10: return (FFTerminalThemeColor){  0, 255,   0, false}; // bright green
        case 11: return (FFTerminalThemeColor){255, 255,   0, false}; // bright yellow
        case 12: return (FFTerminalThemeColor){ 92,  92, 255, false}; // bright blue
        case 13: return (FFTerminalThemeColor){255,   0, 255, false}; // bright magenta
        case 14: return (FFTerminalThemeColor){  0, 255, 255, false}; // bright cyan
        case 15: return (FFTerminalThemeColor){255, 255, 255, false}; // bright white

        default: return (FFTerminalThemeColor){  0,   0,   0, false}; // invalid
    }
}

static bool detectByEnv(FFTerminalThemeResult* result)
{
    const char* color = getenv("COLORFGBG"); // 7;0

    if (!ffStrSet(color))
        return false;

    int f, g;
    if (sscanf(color, "%d;%d", &f, &g) != 2)
        return false;

    result->fg = fgbgToColor(f);
    result->bg = fgbgToColor(g);
    return true;
}

static inline bool detectColor(FFTerminalThemeResult* result, bool forceEnv)
{
    if (!forceEnv && detectByEscapeCode(result))
        return true;

    return detectByEnv(result);
}

bool ffDetectTerminalTheme(FFTerminalThemeResult* result, bool forceEnv)
{
    if (!detectColor(result, forceEnv)) return false;
    result->fg.dark = result->fg.r * 299 + result->fg.g * 587 + result->fg.b * 114 < 128000;
    result->bg.dark = result->bg.r * 299 + result->bg.g * 587 + result->bg.b * 114 < 128000;
    return true;
}
