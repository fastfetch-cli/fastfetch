#include "fastfetch.h"
#include "wmtheme.h"
#include "util/windows/registry.h"

const char* colorHexToString(DWORD hex)
{
    switch(hex)
    {
        case 0x696cc3: return "Yellow gold";
        case 0xff8c00: return "Gold";
        case 0xf7630c: return "Orange bright";
        case 0xca5010: return "Orange dark";
        case 0xda3b01: return "Rust";
        case 0xef6950: return "Pale rust";
        case 0xd13438: return "Brick red";
        case 0xff4343: return "Mod red";
        case 0xe74856: return "Pale red";
        case 0xe81123: return "Red";
        case 0xea005e: return "Rose bright";
        case 0xc30052: return "Rose";
        case 0xe3008c: return "Plum light";
        case 0xbf0077: return "Plum";
        case 0xc239b3: return "Orchid light";
        case 0x9a0089: return "Orchid";
        case 0x0078d4: return "Blue";
        case 0x0063b1: return "Navy blue";
        case 0x8d8bd7: return "Purple shadow";
        case 0x6b69d6: return "Purple shadow dark";
        case 0x8764b8: return "Iris pastel";
        case 0x744da9: return "Iris Spring";
        case 0xb146c2: return "Violet red light";
        case 0x881798: return "Violet red";
        case 0x0099bc: return "Cool blue bright";
        case 0x2d7d9a: return "Cool blue";
        case 0x00b7c3: return "Seafoam";
        case 0x038387: return "Seafoam teal";
        case 0x00b294: return "Mint light";
        case 0x018574: return "Mint dark";
        case 0x00cc6a: return "Turf green";
        case 0x10893e: return "Sport green";
        case 0x7a7574: return "Gray";
        case 0x5d5a58: return "Gray brown";
        case 0x68768a: return "Steel blue";
        case 0x515c6b: return "Metal blue";
        case 0x567c73: return "Pale moss";
        case 0x486860: return "Moss";
        case 0x498205: return "Meadow green";
        case 0x107c10: return "Green";
        case 0x767676: return "Overcast";
        case 0x4c4a48: return "Storm";
        case 0x69797e: return "Blue gray";
        case 0x4a5459: return "Gray dark";
        case 0x647c64: return "Liddy green";
        case 0x4c574e: return "Sage";
        case 0x807143: return "Camouflage desert";
        case 0x766c59: return "Camouflage";
        case 0x000000: return "Black";
        case 0xFFFFFF: return "White";
        default: return NULL;
    }
}

bool ffDetectWmTheme(FFstrbuf* themeOrError)
{
    {
        FF_HKEY_AUTO_DESTROY hKey = NULL;
        if(ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes", &hKey, NULL))
        {
            FF_STRBUF_AUTO_DESTROY theme = ffStrbufCreate();
            if(ffRegReadStrbuf(hKey, L"CurrentTheme", &theme, NULL))
            {
                ffStrbufSubstrBeforeLastC(&theme, '.');
                ffStrbufSubstrAfterLastC(&theme, '\\');
                if(isalpha(theme.chars[0]))
                    theme.chars[0] = (char)toupper(theme.chars[0]);

                ffStrbufAppendF(themeOrError, "%s", theme.chars);
            }
        }
    }

    do {
        uint32_t rgbColor;
        uint32_t bgrColor;
        DWORD bufSize = sizeof(bgrColor);
        if(RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\DWM", L"AccentColor", RRF_RT_REG_DWORD, NULL, &bgrColor, &bufSize) == ERROR_SUCCESS)
            rgbColor = ((bgrColor & 0xFF) << 16) | (bgrColor & 0xFF00) | ((bgrColor >> 16) & 0xFF);
        else if(RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\DWM", L"ColorizationColor", RRF_RT_REG_DWORD, NULL, &rgbColor, &bufSize) == ERROR_SUCCESS)
            rgbColor &= 0xFFFFFF;
        else
            break;

        if(themeOrError->length > 0) ffStrbufAppendS(themeOrError, " - ");
        const char* text = colorHexToString(rgbColor);
        if(text)
            ffStrbufAppendS(themeOrError, text);
        else
            ffStrbufAppendF(themeOrError, "#%06lX", (long)rgbColor);
    } while (false);

    {
        FF_HKEY_AUTO_DESTROY hKey = NULL;
        if(ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", &hKey, NULL))
        {
            uint32_t system = 1, apps = 1;
            if (ffRegReadUint(hKey, L"SystemUsesLightTheme", &system, NULL) && ffRegReadUint(hKey, L"AppsUseLightTheme", &apps, NULL))
            {
                bool paren = themeOrError->length > 0;
                if (paren)
                    ffStrbufAppendS(themeOrError, " (");
                ffStrbufAppendF(themeOrError, "System: %s, Apps: %s", system ? "Light" : "Dark", apps ? "Light" : "Dark");
                if (paren)
                    ffStrbufAppendC(themeOrError, ')');
            }
        }
    }

    if(themeOrError->length == 0)
    {
        ffStrbufAppendS(themeOrError, "Failed to find current theme");
        return false;
    }
    return true;
}
