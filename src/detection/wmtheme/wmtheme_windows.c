#include "fastfetch.h"
#include "wmtheme.h"
#include "util/windows/register.h"

bool ffDetectWmTheme(FFinstance* instance, FFstrbuf* themeOrError)
{
    FF_UNUSED(instance);

    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", &hKey, NULL))
    {
        uint32_t SystemUsesLightTheme = 1;
        if(!ffRegReadUint(hKey, L"SystemUsesLightTheme", &SystemUsesLightTheme, themeOrError))
            return false;

        uint32_t AppsUsesLightTheme = 1;
        if(!ffRegReadUint(hKey, L"AppsUseLightTheme", &AppsUsesLightTheme, themeOrError))
            return false;

        ffStrbufAppendF(themeOrError, "System - %s, Apps - %s", SystemUsesLightTheme ? "Light" : "Dark", AppsUsesLightTheme ? "Light" : "Dark");

        return true;
    }
    else if(ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes", &hKey, NULL))
    {
        if(!ffRegReadStrbuf(hKey, L"CurrentTheme", themeOrError, themeOrError))
            return false;

        ffStrbufSubstrBeforeLastC(themeOrError, '.');
        ffStrbufSubstrAfterLastC(themeOrError, '\\');
        if (isalpha(themeOrError->chars[0]))
            themeOrError->chars[0] = (char)toupper(themeOrError->chars[0]);

        return true;
    }
    else
    {
        ffStrbufAppendS(themeOrError, "Failed to find current theme");
        return false;
    }
}
