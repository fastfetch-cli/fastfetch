#include "os.h"
#include "common/strutil.h"
#include "common/windows/registry.h"
#include "common/windows/unicode.h"

#include <windows.h>

PWSTR WINAPI BrandingFormatString(PCWSTR format);

static bool getCodeName(FFOSResult* os) {
    FF_AUTO_CLOSE_FD HANDLE hKey = nullptr;
    if (!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", &hKey, nullptr)) {
        return false;
    }

    if (!ffRegReadStrbuf(hKey, L"DisplayVersion", &os->codename, nullptr)) {
        if (!ffRegReadStrbuf(hKey, L"CSDVersion", &os->codename, nullptr)) {    // For Windows 7 and Windows 8
            if (!ffRegReadStrbuf(hKey, L"ReleaseId", &os->codename, nullptr)) { // For old Windows 10
                return false;
            }
        }
    }

    return true;
}

void ffDetectOSImpl(FFOSResult* os) {
    // https://dennisbabkin.com/blog/?t=how-to-tell-the-real-version-of-windows-your-app-is-running-on#ver_string

    // Reads a single branding field from winbrand.dll. The buffer is allocated with
    // GlobalAlloc(GMEM_ZEROINIT) by winbrand, so it is ours to release.
    //
    // Fields offered by winbrand.dll (enumerated in basebrd.dll's RES_METADATA resource):
    //   %WINDOWS_GENERIC%   the OS family, independent of edition and release ("Windows")
    //   %WINDOWS_SHORT%     family + release, but hardcoded to "Windows 10" since Windows 10
    //   %WINDOWS_LONG%      "<family> <release> <edition> [...]" ("Windows 11 Pro Insider Preview")
    //   %WINDOWS_PRODUCT%   the product of the running edition
    //   %WINDOWS_COPYRIGHT%, %MICROSOFT_COMPANYNAME%, %MICROSOFT_ACCOUNT(S)%
    // There is deliberately no field for the bare release number: basebrd.dll stores it
    // inside the same per-edition resource as the rest of %WINDOWS_LONG%
    // ("Windows 11 Pro%1%2"), so the release can only be peeled off that string.
    const wchar_t* rawName = BrandingFormatString(L"%WINDOWS_LONG%");
    ffStrbufSetWS(&os->variant, rawName);
    GlobalFree((HGLOBAL) rawName);
    ffStrbufSet(&os->prettyName, &os->variant);
    ffStrbufTrimRight(&os->variant, ' ');

    // WMI returns the "Microsoft" prefix while BrandingFormatString doesn't. Make them consistent.
    if (ffStrbufStartsWithS(&os->variant, "Microsoft ")) {
        ffStrbufSubstrAfter(&os->variant, strlen("Microsoft ") - 1);
    }

    if (os->variant.length == 0) // Windows PE?
    {
        FF_AUTO_CLOSE_FD HANDLE hKey = nullptr;
        if (ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", &hKey, nullptr)) {
            ffRegReadStrbuf(hKey, L"ProductName", &os->variant, nullptr);
        }
    }

    ffStrbufSet(&os->prettyName, &os->variant);

    if (ffStrbufStartsWithS(&os->variant, "Windows ")) {
        ffStrbufAppendS(&os->name, "Windows");

        ffStrbufSubstrAfter(&os->variant, strlen("Windows ") - 1);

        if (ffStrbufStartsWithS(&os->variant, "Server ")) {
            ffStrbufAppendS(&os->name, " Server");
            ffStrbufSubstrAfter(&os->variant, strlen(" Server") - 1);
        } else if (ffStrbufStartsWithS(&os->variant, "Embedded ")) {
            ffStrbufAppendS(&os->name, " Embedded");
            ffStrbufSubstrAfter(&os->variant, strlen(" Embedded") - 1);
        }

        if (ffStrbufStartsWithIgnCaseS(&os->variant, "(TM) ")) {
            ffStrbufSubstrAfter(&os->variant, strlen(" (TM)") - 1);
        }

        uint32_t index = ffStrbufFirstIndexC(&os->variant, ' ');
        ffStrbufAppendNS(&os->version, index, os->variant.chars);
        ffStrbufSubstrAfter(&os->variant, index);

        // Windows Server 20xx Rx
        if (ffStrbufEndsWithC(&os->name, 'r')) {
            if (os->variant.chars[0] == 'R' &&
                ffCharIsDigit(os->variant.chars[1]) &&
                (os->variant.chars[2] == '\0' || os->variant.chars[2] == ' ')) {
                ffStrbufAppendF(&os->version, " R%c", os->variant.chars[1]);
                ffStrbufSubstrAfter(&os->variant, strlen("Rx ") - 1);
            }
        }
    } else {
        // Unknown Windows name, please report this
        ffStrbufAppend(&os->name, &os->variant);
        ffStrbufClear(&os->variant);
    }

    ffStrbufAppendF(&os->id, "%s %s", os->name.chars, os->version.chars);
    ffStrbufSetStatic(&os->idLike, "Windows");

    if (getCodeName(os) && os->codename.length > 0) {
        ffStrbufAppendF(&os->prettyName, " (%s)", os->codename.chars);
    }
}
