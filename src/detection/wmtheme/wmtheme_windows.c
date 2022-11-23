#include "fastfetch.h"
#include "wmtheme.h"
#include "util/windows/register.h"

bool ffDetectWmTheme(FFinstance* instance, FFstrbuf* themeOrError)
{
    FF_UNUSED(instance);

    {
        uint32_t bgrColor;
        DWORD bufSize = sizeof(bgrColor);
        if(RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\DWM", L"AccentColor", RRF_RT_REG_DWORD, NULL, &bgrColor, &bufSize) == ERROR_SUCCESS)
            ffStrbufAppendF(themeOrError, "Accent Color - #%02X%02X%02X", bgrColor & 0xFF, (bgrColor >> 8) & 0xFF, (bgrColor >> 16) & 0xFF);
    }

    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", &hKey, NULL))
    {
        uint32_t value = 1;
        if(ffRegReadUint(hKey, L"SystemUsesLightTheme", &value, NULL))
        {
            if(themeOrError->length > 0) ffStrbufAppendS(themeOrError, ", ");
            ffStrbufAppendF(themeOrError, "System - %s", value ? "Light" : "Dark");
        }

        if(ffRegReadUint(hKey, L"AppsUseLightTheme", &value, NULL))
        {
            if(themeOrError->length > 0) ffStrbufAppendS(themeOrError, ", ");
            ffStrbufAppendF(themeOrError, "Apps - %s", value ? "Light" : "Dark");
        }
    }
    else if(ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes", &hKey, NULL))
    {
        FF_STRBUF_AUTO_DESTROY theme;
        ffStrbufInit(&theme);
        if(ffRegReadStrbuf(hKey, L"CurrentTheme", &theme, NULL))
        {
            ffStrbufSubstrBeforeLastC(themeOrError, '.');
            ffStrbufSubstrAfterLastC(themeOrError, '\\');
            if(isalpha(themeOrError->chars[0]))
                themeOrError->chars[0] = (char)toupper(themeOrError->chars[0]);
            if(themeOrError->length > 0) ffStrbufAppendS(themeOrError, ", ");
            ffStrbufAppendF(themeOrError, "Theme - %s", theme.chars);
        }
    }

    if(themeOrError->length == 0)
    {
        ffStrbufAppendS(themeOrError, "Failed to find current theme");
        return false;
    }
    return true;
}
