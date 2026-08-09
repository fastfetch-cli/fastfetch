#include "detection/locale/locale.h"
#include "common/apple/cf_helpers.h"

const char* ffDetectLocale(FFstrbuf* result) {
    // Read the system-wide locale preference (equivalent to `defaults read -g AppleLocale`),
    // which is NOT affected by user environment variables (LANG, LC_ALL, etc.)
    FF_CFTYPE_AUTO_RELEASE CFStringRef appleLocale = CFPreferencesCopyAppValue(CFSTR("AppleLocale"), kCFPreferencesAnyApplication);
    if (!appleLocale) {
        return "Failed to read AppleLocale";
    }

    return ffCfStrGetString(appleLocale, result);
}
