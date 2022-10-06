#include "fastfetch.h"
#include "wmtheme.h"

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

bool ffDetectWmTheme(FFinstance* instance, FFstrbuf* themeOrError)
{
    FF_UNUSED(instance);

    HKEY hKey;
    if(RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        ffStrbufAppendS(themeOrError, "RegOpenKeyExW() failed");
        return false;
    }

    bool result = true;
    int SystemUsesLightTheme = 1;
    DWORD bufSize = sizeof(SystemUsesLightTheme);

    if(RegQueryValueExW(hKey, L"SystemUsesLightTheme", NULL, NULL, (LPBYTE)&SystemUsesLightTheme, &bufSize) != ERROR_SUCCESS)
    {
        ffStrbufAppendS(themeOrError, "RegOpenKeyExW(SystemUsesLightTheme) failed");
        goto exit;
    }

    int AppsUsesLightTheme = 1;
    bufSize = sizeof(AppsUsesLightTheme);
    if(RegQueryValueExW(hKey, L"AppsUseLightTheme", NULL, NULL, (LPBYTE)&AppsUsesLightTheme, &bufSize) != ERROR_SUCCESS)
    {
        ffStrbufAppendS(themeOrError, "RegOpenKeyExW(AppsUseLightTheme) failed");
        goto exit;
    }

    ffStrbufAppendF(themeOrError, "System - %s, Apps - %s", SystemUsesLightTheme ? "Light" : "Dark", AppsUsesLightTheme ? "Light" : "Dark");

exit:
    RegCloseKey(hKey);
    return result;
}
