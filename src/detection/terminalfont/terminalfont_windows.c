#include "common/properties.h"
#include "detection/terminalshell/terminalshell.h"
#include "terminalfont.h"

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

static void detectMintty(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY fontName;
    ffStrbufInit(&fontName);

    FF_STRBUF_AUTO_DESTROY fontSize;
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
}

static inline void wrapRegCloseKey(HKEY* phKey)
{
    if(*phKey)
        RegCloseKey(*phKey);
}

static void detectConhost(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FF_UNUSED(instance);

    //Current font of conhost doesn't seem to be detectable, we detect default font instead

    HKEY __attribute__((__cleanup__(wrapRegCloseKey))) hKey = NULL;
    if(RegOpenKeyExW(HKEY_CURRENT_USER, L"Console", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        ffStrbufAppendS(&terminalFont->error, "RegOpenKeyExW() failed");
        return;
    }

    DWORD bufSize;

    char fontName[128];
    bufSize = sizeof(fontName);
    if(RegGetValueA(hKey, NULL, "FaceName", RRF_RT_REG_SZ, NULL, fontName, &bufSize) != ERROR_SUCCESS)
    {
        ffStrbufAppendS(&terminalFont->error, "RegGetValueA(FaceName) failed");
        return;
    }

    uint32_t fontSizeNum = 0;
    bufSize = sizeof(fontSizeNum);
    if(RegGetValueW(hKey, NULL, L"FontSize", RRF_RT_DWORD, NULL, &fontSizeNum, &bufSize) != ERROR_SUCCESS)
    {
        ffStrbufAppendS(&terminalFont->error, "RegGetValueW(FontSize) failed");
        return;
    }

    char fontSize[16];
    _ultoa((unsigned long)(fontSizeNum >> 16), fontSize, 10);

    ffFontInitValues(&terminalFont->font, fontName, fontSize);
}

void ffDetectTerminalFontPlatform(const FFinstance* instance, const FFTerminalShellResult* terminalShell, FFTerminalFontResult* terminalFont)
{
    if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "mintty") == 0)
        detectMintty(instance, terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "conhost.exe") == 0)
        detectConhost(instance, terminalFont);
}
