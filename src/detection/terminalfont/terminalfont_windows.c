#include "terminalfont.h"
#include "common/parsing.h"
#include "detection/terminalshell/terminalshell.h"

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

static void detectMintty(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FFstrbuf fontName;
    ffStrbufInit(&fontName);

    FFstrbuf fontSize;
    ffStrbufInit(&fontSize);

    ffParsePropFileHomeValues(instance, ".minttyrc", 2, (FFpropquery[]) {
        {"Font=", &fontName},
        {"FontHeight=", &fontSize}
    });
    if(fontName.length == 0)
        ffStrbufAppendS(&fontName, "Lucida Console");
    if(fontSize.length == 0)
        ffStrbufAppendC(&fontSize, '9');

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&fontSize);
}

static void detectConhost(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FF_UNUSED(instance);

    //Current font of conhost doesn't seem to be detectable, we detect default font instead

    HKEY hKey;
    if(RegOpenKeyExW(HKEY_CURRENT_USER, L"Console", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        ffStrbufAppendS(&terminalFont->error, "RegOpenKeyExW() failed");
        return;
    }

    DWORD bufSize;

    wchar_t fontNameW[64];
    bufSize = sizeof(fontNameW);
    if(RegQueryValueExW(hKey, L"FaceName", NULL, NULL, (LPBYTE)fontNameW, &bufSize) != ERROR_SUCCESS)
    {
        ffStrbufAppendS(&terminalFont->error, "RegOpenKeyExW(FaceName) failed");
        goto exit;
    }
    fontNameW[bufSize] = '\0';

    char fontNameA[128];
    int fontNameALen = WideCharToMultiByte(CP_UTF8, 0, fontNameW, (int)(bufSize / 2), fontNameA, sizeof(fontNameA), NULL, NULL);
    fontNameA[fontNameALen] = '\0';

    uint32_t fontSizeNum = 0;
    bufSize = sizeof(fontSizeNum);
    if(RegQueryValueExW(hKey, L"fontSize", NULL, NULL, (LPBYTE)&fontSizeNum, &bufSize) != ERROR_SUCCESS)
    {
        ffStrbufAppendS(&terminalFont->error, "RegOpenKeyExW(fontSize) failed");
        goto exit;
    }

    char fontSize[16];
    snprintf(fontSize, sizeof(fontSize), "%u", (fontSizeNum >> 16));

    ffFontInitValues(&terminalFont->font, fontNameA, fontSize);

exit:
    RegCloseKey(hKey);
}

void ffDetectTerminalFontPlatform(const FFinstance* instance, const FFTerminalShellResult* terminalShell, FFTerminalFontResult* terminalFont)
{
    if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "mintty") == 0)
        detectMintty(instance, terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "conhost.exe") == 0)
        detectConhost(instance, terminalFont);
}
