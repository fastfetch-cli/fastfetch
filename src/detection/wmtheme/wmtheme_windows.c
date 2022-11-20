#include "fastfetch.h"
#include "wmtheme.h"

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

static inline void wrapRegCloseKey(HKEY* phKey)
{
    if(*phKey)
        RegCloseKey(*phKey);
}

bool ffDetectWmTheme(FFinstance* instance, FFstrbuf* themeOrError)
{
    FF_UNUSED(instance);

    HKEY __attribute__((__cleanup__(wrapRegCloseKey))) hKey = NULL;
    if(RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        int SystemUsesLightTheme = 1;
        DWORD bufSize = sizeof(SystemUsesLightTheme);

        if(RegGetValueW(hKey, NULL, L"SystemUsesLightTheme", RRF_RT_DWORD, NULL, &SystemUsesLightTheme, &bufSize) != ERROR_SUCCESS)
        {
            ffStrbufAppendS(themeOrError, "RegGetValueW(SystemUsesLightTheme) failed");
            return false;
        }

        int AppsUsesLightTheme = 1;
        bufSize = sizeof(AppsUsesLightTheme);
        if(RegGetValueW(hKey, NULL, L"AppsUseLightTheme", RRF_RT_DWORD, NULL, &AppsUsesLightTheme, &bufSize) != ERROR_SUCCESS)
        {
            ffStrbufAppendS(themeOrError, "RegGetValueW(AppsUseLightTheme) failed");
            return false;
        }

        ffStrbufAppendF(themeOrError, "System - %s, Apps - %s", SystemUsesLightTheme ? "Light" : "Dark", AppsUsesLightTheme ? "Light" : "Dark");

        return true;
    }
    else if(RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD length = 0;
        if(RegGetValueA(hKey, NULL, "CurrentTheme", RRF_RT_REG_SZ, NULL, NULL, &length) != ERROR_SUCCESS)
        {
            ffStrbufAppendS(themeOrError, "RegGetValueA(CurrentTheme, NULL) failed");
            return false;
        }

        ffStrbufEnsureFree(themeOrError, length);
        if(RegGetValueA(hKey, NULL, "CurrentTheme", RRF_RT_REG_SZ, NULL, themeOrError->chars, &length) != ERROR_SUCCESS)
        {
            ffStrbufAppendS(themeOrError, "RegGetValueA(CurrentTheme) failed");
            return false;
        }

        themeOrError->length = length;
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
