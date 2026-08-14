#include "detection/locale/locale.h"
#include "common/windows/unicode.h"

#include <winnls.h>

const char* ffDetectLocale(FFstrbuf* result) {
    ffStrbufAppendS(result, getenv("LC_ALL"));
    if (result->length > 0) {
        return nullptr;
    }

    ffStrbufAppendS(result, getenv("LANG")); // Available in MSYS2 and Cygwin
    if (result->length > 0) {
        return nullptr;
    }

    wchar_t name[LOCALE_NAME_MAX_LENGTH];
    int size = GetUserDefaultLocaleName(name, LOCALE_NAME_MAX_LENGTH);
    if (size <= 1) { // including '\0'
        return "GetUserDefaultLocaleName() failed";
    }
    ffStrbufAppendNWS(result, (uint32_t) size - 1, name);
    if (result->length > 2 && result->chars[2] == '-') { // Windows uses '-' instead of '_'
        result->chars[2] = '_';
    }

    uint32_t codePage = instance.state.platform.initCP;
    if (__builtin_expect(codePage != 0, true)) {
        ffStrbufAppendC(result, '.');

        switch (codePage) {
            case CP_UTF8:
                ffStrbufAppendS(result, "UTF-8");
                break;
            case CP_UTF7:
                ffStrbufAppendS(result, "UTF-7");
                break;
            case 1200:
                ffStrbufAppendS(result, "UTF-16LE");
                break;
            case 1201:
                ffStrbufAppendS(result, "UTF-16BE");
                break;
            case 936:
                ffStrbufAppendS(result, "GBK");
                break;
            case 54936:
                ffStrbufAppendS(result, "GB18030");
                break;
            case 950:
                ffStrbufAppendS(result, "BIG5");
                break;
            case 932:
                ffStrbufAppendS(result, "Shift_JIS");
                break;
            case 949:
                ffStrbufAppendS(result, "EUC-KR");
                break;
            case 20932:
                ffStrbufAppendS(result, "EUC-JP");
                break;

            case 20866:
                ffStrbufAppendS(result, "KOI8-R");
                break;
            case 21866:
                ffStrbufAppendS(result, "KOI8-U");
                break;
            case 20127:
                ffStrbufAppendS(result, "US-ASCII");
                break;

            case 874:
            case 1250 ... 1258:
                ffStrbufAppendS(result, "Windows-");
                ffStrbufAppendUInt(result, codePage);
                break;

            default:
                ffStrbufAppendS(result, "CP");
                ffStrbufAppendUInt(result, codePage);
                break;
        }
    }

    return nullptr;
}
