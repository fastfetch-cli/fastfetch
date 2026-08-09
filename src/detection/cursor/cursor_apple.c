#include "cursor.h"
#include "common/apple/cf_helpers.h"

static bool appendColor(FFstrbuf* str, CFDictionaryRef dict) {
    double r, g, b, a;
    if (
        ffCfDictGetDouble(dict, CFSTR("red"), &r) ||
        ffCfDictGetDouble(dict, CFSTR("green"), &g) ||
        ffCfDictGetDouble(dict, CFSTR("blue"), &b) ||
        ffCfDictGetDouble(dict, CFSTR("alpha"), &a)) {
        return false;
    }

    r = r * 255 + .5;
    g = g * 255 + .5;
    b = b * 255 + .5;
    a = a * 255 + .5;

    uint32_t color = ((uint32_t) r << 24) | ((uint32_t) g << 16) | ((uint32_t) b << 8) | ((uint32_t) a);

    // clang-format off
    switch (color)
    {
        case 0x000000FF: ffStrbufAppendS(str, "Black"); break;
        case 0x0433FFFF: ffStrbufAppendS(str, "Blue"); break;
        case 0xAA7942FF: ffStrbufAppendS(str, "Brown"); break;
        case 0x00FDFFFF: ffStrbufAppendS(str, "Cyan"); break;
        case 0x00F900FF: ffStrbufAppendS(str, "Green"); break;
        case 0xFF40FFFF: ffStrbufAppendS(str, "Magenta"); break;
        case 0xFF9300FF: ffStrbufAppendS(str, "Orange"); break;
        case 0x942192FF: ffStrbufAppendS(str, "Purple"); break;
        case 0xFF2600FF: ffStrbufAppendS(str, "Red"); break;
        case 0xFFFB00FF: ffStrbufAppendS(str, "Yellow"); break;
        case 0xFFFFFFFF: ffStrbufAppendS(str, "White"); break;
        case 0x00000000: ffStrbufAppendS(str, "Transparent"); break;
        default: ffStrbufAppendF(str, "#%08X", color); break;
    }
    // clang-format on

    return true;
}

void ffDetectCursor(FFCursorResult* result) {
    // Read via cfprefsd (equivalent to `defaults read com.apple.universalaccess <key>`)
    ffStrbufAppendS(&result->theme, "Fill - ");
    {
        FF_CFTYPE_AUTO_RELEASE CFTypeRef color = CFPreferencesCopyAppValue(CFSTR("cursorFill"), CFSTR("com.apple.universalaccess"));
        if (!color || CFGetTypeID(color) != CFDictionaryGetTypeID() || !appendColor(&result->theme, (CFDictionaryRef) color)) {
            ffStrbufAppendS(&result->theme, "Black");
        }
    }

    ffStrbufAppendS(&result->theme, ", Outline - ");
    {
        FF_CFTYPE_AUTO_RELEASE CFTypeRef color = CFPreferencesCopyAppValue(CFSTR("cursorOutline"), CFSTR("com.apple.universalaccess"));
        if (!color || CFGetTypeID(color) != CFDictionaryGetTypeID() || !appendColor(&result->theme, (CFDictionaryRef) color)) {
            ffStrbufAppendS(&result->theme, "White");
        }
    }

    {
        FF_CFTYPE_AUTO_RELEASE CFTypeRef mouseDriverCursorSize = CFPreferencesCopyAppValue(CFSTR("mouseDriverCursorSize"), CFSTR("com.apple.universalaccess"));
        double size = 32;
        if (mouseDriverCursorSize && ffCfNumGetDouble(mouseDriverCursorSize, &size) == nullptr) {
            ffStrbufAppendUInt(&result->size, (uint32_t) (size * 32 + 0.5));
        } else {
            ffStrbufAppendS(&result->size, "32");
        }
    }
}
