#include "terminalfont.h"
#include "common/settings.h"
#include "common/properties.h"
#include "common/parsing.h"
#include "detection/terminalshell/terminalshell.h"
#include "detection/displayserver/displayserver.h"

static const char* getSystemMonospaceFont(const FFinstance* instance)
{
    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, "Cinnamon") == 0)
    {
        const char* systemMonospaceFont = ffSettingsGet(instance, "/org/cinnamon/desktop/interface/monospace-font-name", "org.cinnamon.desktop.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(systemMonospaceFont))
            return systemMonospaceFont;
    }
    else if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, "Mate") == 0)
    {
        const char* systemMonospaceFont = ffSettingsGet(instance, "/org/mate/interface/monospace-font-name", "org.mate.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(systemMonospaceFont))
            return systemMonospaceFont;
    }

    return ffSettingsGet(instance, "/org/gnome/desktop/interface/monospace-font-name", "org.gnome.desktop.interface", NULL, "monospace-font-name", FF_VARIANT_TYPE_STRING).strValue;
}

static void detectFromGSettings(const FFinstance* instance, char* profilePath, char* profileList, char* profile, FFTerminalFontResult* terminalFont)
{
    const char* defaultProfile = ffSettingsGetGSettings(instance, profileList, NULL, "default", FF_VARIANT_TYPE_STRING).strValue;
    if(!ffStrSet(defaultProfile))
    {
        ffStrbufAppendF(&terminalFont->error, "Could not get default profile from gsettings: %s", profileList);
        return;
    }

    FFstrbuf path;
    ffStrbufInitA(&path, 128);
    ffStrbufAppendS(&path, profilePath);
    ffStrbufAppendS(&path, defaultProfile);
    ffStrbufAppendC(&path, '/');

    if(!ffSettingsGetGSettings(instance, profile, path.chars, "use-system-font", FF_VARIANT_TYPE_BOOL).boolValue)
    {
        const char* fontName = ffSettingsGetGSettings(instance, profile, path.chars, "font", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(fontName))
            ffFontInitPango(&terminalFont->font, fontName);
        else
            ffStrbufAppendF(&terminalFont->error, "Couldn't get terminal font from GSettings (%s::%s::font)", profile, path.chars);
    }
    else
    {
        const char* fontName = getSystemMonospaceFont(instance);
        if(ffStrSet(fontName))
            ffFontInitPango(&terminalFont->font, fontName);
        else
            ffStrbufAppendS(&terminalFont->error, "Could't get system monospace font name from GSettings / DConf");
    }

    ffStrbufDestroy(&path);
}

static void detectFromConfigFile(const FFinstance* instance, const char* configFile, const char* start, FFTerminalFontResult* terminalFont)
{
    FFstrbuf fontName;
    ffStrbufInit(&fontName);
    ffParsePropFileConfig(instance, configFile, start, &fontName);

    if(fontName.length == 0)
        ffStrbufAppendF(&terminalFont->error, "Couldn't find %s in .config/%s", start, configFile);
    else
        ffFontInitPango(&terminalFont->font, fontName.chars);

    ffStrbufDestroy(&fontName);
}

static void detectKonsole(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FFstrbuf profile;
    ffStrbufInit(&profile);
    ffParsePropFileConfig(instance, "konsolerc", "DefaultProfile =", &profile);

    if(profile.length == 0)
    {
        ffStrbufAppendS(&terminalFont->error, "Couldn't find \"DefaultProfile=%[^\\n]\" in \".config/konsolerc\"");
        ffStrbufDestroy(&profile);
        return;
    }

    FFstrbuf profilePath;
    ffStrbufInitA(&profilePath, 64);
    ffStrbufAppendS(&profilePath, ".local/share/konsole/");
    ffStrbufAppend(&profilePath, &profile);

    ffStrbufDestroy(&profile);

    FFstrbuf fontName;
    ffStrbufInit(&fontName);
    ffParsePropFileHome(instance, profilePath.chars, "Font =", &fontName);

    if(fontName.length == 0)
        ffStrbufAppendF(&terminalFont->error, "Couldn't find \"Font=%%[^\\n]\" in \"%s\"", profilePath.chars);
    else
        ffFontInitQt(&terminalFont->font, fontName.chars);

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&profilePath);
}

static void detectXFCETerminal(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FFstrbuf useSysFont;
    ffStrbufInit(&useSysFont);

    FFstrbuf fontName;
    ffStrbufInit(&fontName);

    ffParsePropFileConfigValues(instance, "xfce4/terminal/terminalrc", 2, (FFpropquery[]) {
        {"FontUseSystem = ", &useSysFont},
        {"FontName = ", &fontName}
    });

    if(useSysFont.length == 0 || ffStrbufIgnCaseCompS(&useSysFont, "false") == 0)
    {
        if(fontName.length == 0)
            ffStrbufAppendS(&terminalFont->error, "Couldn't find FontName in .config/xfce4/terminal/terminalrc");
        else
            ffFontInitPango(&terminalFont->font, fontName.chars);
    }
    else
    {
        const char* systemFontName = ffSettingsGetXFConf(instance, "xsettings", "/Gtk/MonospaceFontName", FF_VARIANT_TYPE_STRING).strValue;
        if(ffStrSet(systemFontName))
            ffFontInitPango(&terminalFont->font, systemFontName);
        else
            ffStrbufAppendS(&terminalFont->error, "Couldn't find xsettings::/Gtk/MonospaceFontName in XFConf");
    }

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&useSysFont);
}

#ifdef FF_HAVE_LIBCJSON

#include "common/library.h"
#include "common/processing.h"

#include <cjson/cJSON.h>
#include <stdlib.h>

typedef struct CJSONData
{
    FF_LIBRARY_SYMBOL(cJSON_Parse)
    FF_LIBRARY_SYMBOL(cJSON_IsObject)
    FF_LIBRARY_SYMBOL(cJSON_GetObjectItemCaseSensitive)
    FF_LIBRARY_SYMBOL(cJSON_IsString)
    FF_LIBRARY_SYMBOL(cJSON_GetStringValue)
    FF_LIBRARY_SYMBOL(cJSON_IsNumber)
    FF_LIBRARY_SYMBOL(cJSON_GetNumberValue)
    FF_LIBRARY_SYMBOL(cJSON_IsArray)
    FF_LIBRARY_SYMBOL(cJSON_Delete)
} CJSONData;

static const char* detectWTProfile(CJSONData* cjsonData, cJSON* profile, FFstrbuf* name, int* size)
{
    if(!cjsonData->ffcJSON_IsObject(profile))
        return "cJSON_IsObject(profile) returns false";

    cJSON* font = cjsonData->ffcJSON_GetObjectItemCaseSensitive(profile, "font");
    if(!cjsonData->ffcJSON_IsObject(font))
        return "cJSON_IsObject(font) returns false";

    if(name->length == 0)
    {
        cJSON* pface = cjsonData->ffcJSON_GetObjectItemCaseSensitive(font, "face");
        if(cjsonData->ffcJSON_IsString(pface))
            ffStrbufAppendS(name, cjsonData->ffcJSON_GetStringValue(pface));
    }
    if(*size < 0)
    {
        cJSON* psize = cjsonData->ffcJSON_GetObjectItemCaseSensitive(font, "size");
        if(cjsonData->ffcJSON_IsNumber(psize))
            *size = (int)cjsonData->ffcJSON_GetNumberValue(psize);
    }
    return NULL;
}

static const char* detectFromWTImpl(const FFinstance* instance, FFstrbuf* content, FFstrbuf* name, int* size)
{
    CJSONData cjsonData;

    FF_LIBRARY_LOAD(libcjson, &instance->config.libcJSON, "dlopen libcjson"FF_LIBRARY_EXTENSION" failed", "libcjson"FF_LIBRARY_EXTENSION, 1)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libcjson, cjsonData, cJSON_Parse)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libcjson, cjsonData, cJSON_IsObject)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libcjson, cjsonData, cJSON_GetObjectItemCaseSensitive)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libcjson, cjsonData, cJSON_IsString)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libcjson, cjsonData, cJSON_GetStringValue)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libcjson, cjsonData, cJSON_IsNumber)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libcjson, cjsonData, cJSON_GetNumberValue)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libcjson, cjsonData, cJSON_IsArray)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libcjson, cjsonData, cJSON_Delete)

    const char* error = NULL;

    cJSON* root = cjsonData.ffcJSON_Parse(content->chars);
    if(!cjsonData.ffcJSON_IsObject(root))
    {
        error = "cJSON_Parse() failed";
        goto exit;
    }

    cJSON* profiles = cjsonData.ffcJSON_GetObjectItemCaseSensitive(root, "profiles");
    if(!cjsonData.ffcJSON_IsObject(profiles))
    {
        error = "cJSON_GetObjectItemCaseSensitive(root, \"profiles\") failed";
        goto exit;
    }

    FFstrbuf wtProfileId;
    ffStrbufInitS(&wtProfileId, getenv("WT_PROFILE_ID"));
    ffStrbufTrim(&wtProfileId, '\'');
    if(wtProfileId.length > 0)
    {
        cJSON* list = cjsonData.ffcJSON_GetObjectItemCaseSensitive(profiles, "list");
        if(cjsonData.ffcJSON_IsArray(list))
        {
            cJSON* profile;
            cJSON_ArrayForEach(profile, list)
            {
                if(!cjsonData.ffcJSON_IsObject(profile))
                    continue;
                cJSON* guid = cjsonData.ffcJSON_GetObjectItemCaseSensitive(profile, "guid");
                if(!cjsonData.ffcJSON_IsString(guid))
                    continue;
                if(ffStrbufCompS(&wtProfileId, cjsonData.ffcJSON_GetStringValue(guid)) == 0)
                {
                    detectWTProfile(&cjsonData, profile, name, size);
                    break;
                }
            }
        }
    }
    ffStrbufDestroy(&wtProfileId);

    cJSON* defaults = cjsonData.ffcJSON_GetObjectItemCaseSensitive(profiles, "defaults");
    detectWTProfile(&cjsonData, defaults, name, size);

    if(name->length == 0)
        ffStrbufSetS(name, "Cascadia Mono");
    if(*size < 0)
        *size = 12;

exit:
    cjsonData.ffcJSON_Delete(root);
    dlclose(libcjson);
    return error;
}

static void detectFromWindowsTeriminal(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    //https://learn.microsoft.com/en-us/windows/terminal/install#settings-json-file
    FFstrbuf json;
    ffStrbufInit(&json);
    const char* error;
    error = ffProcessAppendStdOut(&json, (char* const[]) {
        "cmd.exe",
        "/c",
        //print the file content directly, so we don't need to handle the difference of Windows and POSIX path
        "if exist %LOCALAPPDATA%\\Packages\\Microsoft.WindowsTerminal_8wekyb3d8bbwe\\LocalState\\settings.json "
        "( type %LOCALAPPDATA%\\Packages\\Microsoft.WindowsTerminal_8wekyb3d8bbwe\\LocalState\\settings.json ) "
        "else if exist %LOCALAPPDATA%\\Packages\\Microsoft.WindowsTerminalPreview_8wekyb3d8bbwe\\LocalState\\settings.json "
        "( type %LOCALAPPDATA%\\Packages\\Microsoft.WindowsTerminalPreview_8wekyb3d8bbwe\\LocalState\\settings.json ) "
        "else if exist \"%LOCALAPPDATA%\\Microsoft\\Windows Terminal\\settings.json\" "
        "( type %LOCALAPPDATA%\\Microsoft\\Windows Terminal\\settings.json ) "
        "else ( call )",
        NULL
    });
    if(error)
    {
        ffStrbufAppendS(&terminalFont->error, error);
        ffStrbufDestroy(&json);
        return;
    }
    ffStrbufTrimRight(&json, '\n');
    if(json.length == 0)
    {
        ffStrbufAppendS(&terminalFont->error, "Cannot find file \"settings.json\"");
        ffStrbufDestroy(&json);
        return;
    }

    FFstrbuf name;
    ffStrbufInit(&name);
    int size = -1;
    error = detectFromWTImpl(instance, &json, &name, &size);
    ffStrbufDestroy(&json);

    if(error)
        ffStrbufAppendS(&terminalFont->error, error);
    else
    {
        char sizeStr[16];
        snprintf(sizeStr, sizeof(sizeStr), "%d", size);
        ffFontInitValues(&terminalFont->font, name.chars, sizeStr);
    }

    ffStrbufDestroy(&name);
}

#else

static void detectFromWindowsTeriminal(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FF_UNUSED(instance, terminalFont);
    ffStrbufAppendS(&terminalFont->error, "fastfetch is built without libcjson support");
}

#endif

#if defined(_WIN32) || defined(__MSYS__)
// TODO: move to a separate file

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

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

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

#endif //defined(_WIN32) || defined(__MSYS__)

void ffDetectTerminalFontPlatform(const FFinstance* instance, const FFTerminalShellResult* terminalShell, FFTerminalFontResult* terminalFont)
{
    if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "konsole") == 0)
        detectKonsole(instance, terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "xfce4-terminal") == 0)
        detectXFCETerminal(instance, terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "lxterminal") == 0)
        detectFromConfigFile(instance, "lxterminal/lxterminal.conf", "fontname =", terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "tilix") == 0)
        detectFromGSettings(instance, "/com/gexperts/Tilix/profiles/", "com.gexperts.Tilix.ProfilesList", "com.gexperts.Tilix.Profile", terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "gnome-terminal-") == 0)
        detectFromGSettings(instance, "/org/gnome/terminal/legacy/profiles:/:", "org.gnome.Terminal.ProfilesList", "org.gnome.Terminal.Legacy.Profile", terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "Windows Terminal") == 0 ||
        ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "WindowsTerminal.exe") == 0)
        detectFromWindowsTeriminal(instance, terminalFont);

    #if defined(_WIN32) || defined(__MSYS__)
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "mintty") == 0)
        detectMintty(instance, terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "conhost.exe") == 0)
        detectConhost(instance, terminalFont);
    #endif
}
