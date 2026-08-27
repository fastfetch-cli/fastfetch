#include "fastfetch.h"
#include "wmtheme.h"
#include "common/apple/cf_helpers.h"

bool ffDetectWmTheme(FFstrbuf* themeOrError) {
    // Read via cfprefsd instead of the raw plist file to keep cache consistency with the system
    FF_CFTYPE_AUTO_RELEASE CFTypeRef wmThemeColor = CFPreferencesCopyAppValue(CFSTR("AppleAccentColor"), kCFPreferencesAnyApplication);
    int32_t accentColor = -2; // -2: not set; -1 is Graphite
    if (wmThemeColor && ffCfNumGetInt(wmThemeColor, &accentColor) == nullptr) {
        switch (accentColor) {
            case -1:
                ffStrbufAppendS(themeOrError, "Graphite");
                break;
            case 0:
                ffStrbufAppendS(themeOrError, "Red");
                break;
            case 1:
                ffStrbufAppendS(themeOrError, "Orange");
                break;
            case 2:
                ffStrbufAppendS(themeOrError, "Yellow");
                break;
            case 3:
                ffStrbufAppendS(themeOrError, "Green");
                break;
            case 4:
                ffStrbufAppendS(themeOrError, "Blue");
                break;
            case 5:
                ffStrbufAppendS(themeOrError, "Purple");
                break;
            case 6:
                ffStrbufAppendS(themeOrError, "Pink");
                break;
            default:
                ffStrbufAppendS(themeOrError, "Unknown");
                break;
        }
    } else {
        ffStrbufAppendS(themeOrError, "Multicolor");
    }

    FF_STRBUF_AUTO_DESTROY style = ffStrbufCreate();
    FF_CFTYPE_AUTO_RELEASE CFTypeRef wmTheme = CFPreferencesCopyAppValue(CFSTR("AppleInterfaceStyle"), kCFPreferencesAnyApplication);
    if (wmTheme && ffCfStrGetString(wmTheme, &style) == nullptr) {
        ffStrbufAppendF(themeOrError, " (%s)", style.chars);
    } else {
        ffStrbufAppendS(themeOrError, " (Light)");
    }

    return true;
}
