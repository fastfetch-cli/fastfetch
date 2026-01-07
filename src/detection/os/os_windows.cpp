extern "C" {
#include "os.h"
#include "common/library.h"
#include "common/stringUtils.h"
#include "common/windows/registry.h"
}
#include "common/windows/unicode.hpp"
#include "common/windows/wmi.hpp"

static const char* getOsNameByWmi(FFstrbuf* osName)
{
    FFWmiQuery query(L"SELECT Caption FROM Win32_OperatingSystem");
    if(!query)
        return "Query WMI service failed";

    if(FFWmiRecord record = query.next())
    {
        if(auto vtCaption = record.get(L"Caption"))
        {
            ffStrbufSetWSV(osName, vtCaption.get<std::wstring_view>());
            ffStrbufTrimRight(osName, ' ');
            return NULL;
        }
        return "Get Caption failed";
    }

    return "No WMI result returned";
}

PWSTR WINAPI BrandingFormatString(PCWSTR format);

static bool getCodeName(FFOSResult* os)
{
    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", &hKey, NULL))
        return false;

    if(!ffRegReadStrbuf(hKey, L"DisplayVersion", &os->codename, NULL))
    {
        if (!ffRegReadStrbuf(hKey, L"CSDVersion", &os->codename, NULL)) // For Windows 7 and Windows 8
            if (!ffRegReadStrbuf(hKey, L"ReleaseId", &os->codename, NULL)) // For old Windows 10
                return false;
    }

    return true;
}

static const char* getOsNameByWinbrand(FFstrbuf* osName)
{
    //https://dennisbabkin.com/blog/?t=how-to-tell-the-real-version-of-windows-your-app-is-running-on#ver_string
    FF_LIBRARY_LOAD_MESSAGE(winbrand, "winbrand" FF_LIBRARY_EXTENSION, 1);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(winbrand, BrandingFormatString);

    const wchar_t* rawName = ffBrandingFormatString(L"%WINDOWS_LONG%");
    ffStrbufSetWS(osName, rawName);
    GlobalFree((HGLOBAL)rawName);
    return NULL;
}

extern "C"
void ffDetectOSImpl(FFOSResult* os)
{
    if(getOsNameByWinbrand(&os->variant) && getOsNameByWmi(&os->variant))
        return;

    ffStrbufTrimRight(&os->variant, ' ');

    //WMI returns the "Microsoft" prefix while BrandingFormatString doesn't. Make them consistent.
    if(ffStrbufStartsWithS(&os->variant, "Microsoft "))
        ffStrbufSubstrAfter(&os->variant, strlen("Microsoft ") - 1);

    if(os->variant.length == 0) // Windows PE?
    {
        wchar_t buf[128];
        DWORD bufSize = (DWORD) sizeof(buf); // with trailing '\0'
        if(RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductName", RRF_RT_REG_SZ, NULL, buf, &bufSize) == ERROR_SUCCESS)
        {
            assert(bufSize >= sizeof(wchar_t));
            ffStrbufSetNWS(&os->variant, bufSize / sizeof(wchar_t) - 1, buf);
        }
    }

    ffStrbufSet(&os->prettyName, &os->variant);

    if(ffStrbufStartsWithS(&os->variant, "Windows "))
    {
        ffStrbufAppendS(&os->name, "Windows");

        ffStrbufSubstrAfter(&os->variant, strlen("Windows ") - 1);

        if(ffStrbufStartsWithS(&os->variant, "Server "))
        {
            ffStrbufAppendS(&os->name, " Server");
            ffStrbufSubstrAfter(&os->variant, strlen(" Server") - 1);
        }

        if(ffStrbufStartsWithIgnCaseS(&os->variant, "(TM) "))
            ffStrbufSubstrAfter(&os->variant, strlen(" (TM)") - 1);

        uint32_t index = ffStrbufFirstIndexC(&os->variant, ' ');
        ffStrbufAppendNS(&os->version, index, os->variant.chars);
        ffStrbufSubstrAfter(&os->variant, index);

        // Windows Server 20xx Rx
        if(ffStrbufEndsWithC(&os->name, 'r'))
        {
            if(os->variant.chars[0] == 'R' &&
                ffCharIsDigit(os->variant.chars[1]) &&
                (os->variant.chars[2] == '\0' || os->variant.chars[2] == ' '))
            {
                ffStrbufAppendF(&os->version, " R%c", os->variant.chars[1]);
                ffStrbufSubstrAfter(&os->variant, strlen("Rx ") - 1);
            }
        }
    }
    else
    {
        // Unknown Windows name, please report this
        ffStrbufAppend(&os->name, &os->variant);
        ffStrbufClear(&os->variant);
    }

    ffStrbufAppendF(&os->id, "%s %s", os->name.chars, os->version.chars);
    ffStrbufSetStatic(&os->idLike, "Windows");

    if (getCodeName(os) && os->codename.length > 0)
        ffStrbufAppendF(&os->prettyName, " (%s)", os->codename.chars);
}
