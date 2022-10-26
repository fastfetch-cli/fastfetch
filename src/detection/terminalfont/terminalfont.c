#include "terminalfont.h"
#include "common/properties.h"
#include "common/processing.h"
#include "detection/internal.h"
#include "detection/terminalshell/terminalshell.h"

static void detectAlacritty(const FFinstance* instance, FFTerminalFontResult* terminalFont) {
    FFstrbuf fontName;
    ffStrbufInit(&fontName);

    FFstrbuf fontSize;
    ffStrbufInit(&fontSize);

    FFpropquery fontQuery[] = {
        {"family:", &fontName},
        {"size:", &fontSize},
    };

    // alacritty parses config files in this order
    ffParsePropFileConfigValues(instance, "alacritty/alacritty.yml", 2, fontQuery);
    if(fontName.length == 0 || fontSize.length == 0)
        ffParsePropFileConfigValues(instance, "alacritty.yml", 2, fontQuery);
    if(fontName.length == 0 || fontSize.length == 0)
        ffParsePropFileConfigValues(instance, ".alacritty.yml", 2, fontQuery);

    //by default alacritty uses its own font called alacritty
    if(fontName.length == 0)
        ffStrbufAppendS(&fontName, "alacritty");

    // the default font size is 11
    if(fontSize.length == 0)
        ffStrbufAppendS(&fontSize, "11");

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&fontSize);
}

static void detectTTY(FFTerminalFontResult* terminalFont)
{
    FFstrbuf fontName;
    ffStrbufInit(&fontName);

    ffParsePropFile(FASTFETCH_TARGET_DIR_ETC"/vconsole.conf", "Font =", &fontName);

    if(fontName.length == 0)
    {
        ffStrbufAppendS(&fontName, "VGA default kernel font ");
        ffProcessAppendStdOut(&fontName, (char* const[]){
            "showconsolefont",
            "--info",
            NULL
        });

        ffStrbufTrimRight(&fontName, ' ');
    }

    if(fontName.length > 0)
        ffFontInitCopy(&terminalFont->font, fontName.chars);
    else
        ffStrbufAppendS(&terminalFont->error, "Couldn't find Font in "FASTFETCH_TARGET_DIR_ETC"/vconsole.conf");

    ffStrbufDestroy(&fontName);
}

#if defined(_WIN32) || defined(__linux__)
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

#ifdef _WIN32
    #include "common/io.h"

    #include <shlobj.h>
#endif

static void detectFromWindowsTeriminal(const FFinstance* instance, const FFstrbuf* terminalExe, FFTerminalFontResult* terminalFont)
{
    //https://learn.microsoft.com/en-us/windows/terminal/install#settings-json-file
    FFstrbuf json;
    ffStrbufInit(&json);
    const char* error = NULL;

    #ifdef _WIN32
    if(terminalExe && terminalExe->length > 0)
    {
        char jsonPath[MAX_PATH + 1];
        if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, jsonPath)))
        {
            size_t remaining = sizeof(jsonPath) - strlen(jsonPath) - 1;
            if(ffStrbufContainIgnCaseS(terminalExe, "_8wekyb3d8bbwe\\"))
            {
                // Microsoft Store version
                if(ffStrbufContainIgnCaseS(terminalExe, ".WindowsTerminalPreview_"))
                {
                    // Preview version
                    strncat(jsonPath, "\\Packages\\Microsoft.WindowsTerminalPreview_8wekyb3d8bbwe\\LocalState\\settings.json", remaining);
                    if(!ffAppendFileBuffer(jsonPath, &json))
                        error = "Error reading Windows Terminal Preview settings JSON file";
                }
                else
                {
                    // Stable version
                    strncat(jsonPath, "\\Packages\\Microsoft.WindowsTerminal_8wekyb3d8bbwe\\LocalState\\settings.json", remaining);
                    if(!ffAppendFileBuffer(jsonPath, &json))
                        error = "Error reading Windows Terminal settings JSON file";
                }
            }
            else
            {
                strncat(jsonPath, "\\Microsoft\\Windows Terminal\\settings.json", remaining);
                if(!ffAppendFileBuffer(jsonPath, &json))
                    error = "Error reading Windows Terminal settings JSON file";
            }
        }
    }
    #else
    FF_UNUSED(terminalExe);
    #endif

    if(!error && json.length == 0)
    {
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
    }

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

static void detectFromWindowsTeriminal(const FFinstance* instance, const FFstrbuf* terminalExe, FFTerminalFontResult* terminalFont)
{
    FF_UNUSED(instance, terminalExe, terminalFont);
    ffStrbufAppendS(&terminalFont->error, "fastfetch is built without libcjson support");
}

#endif
#endif //defined(_WIN32) || defined(__linux__)

void ffDetectTerminalFontPlatform(const FFinstance* instance, const FFTerminalShellResult* terminalShell, FFTerminalFontResult* terminalFont);

static bool detectTerminalFontCommon(const FFinstance* instance, const FFTerminalShellResult* terminalShell, FFTerminalFontResult* terminalFont)
{
    if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "alacritty") == 0)
        detectAlacritty(instance, terminalFont);
    else if(ffStrbufStartsWithIgnCaseS(&terminalShell->terminalExe, "/dev/tty"))
        detectTTY(terminalFont);

    #if defined(_WIN32) || defined(__linux__)
    //Used by both Linux (WSL) and Windows
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "Windows Terminal") == 0 ||
        ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "WindowsTerminal.exe") == 0)
        detectFromWindowsTeriminal(instance, &terminalShell->terminalExe, terminalFont);
    #endif

    else
        return false;

    return true;
}

const FFTerminalFontResult* ffDetectTerminalFont(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFTerminalFontResult,
        ffStrbufInitA(&result.error, 0);

        const FFTerminalShellResult* terminalShell = ffDetectTerminalShell(instance);

        if(terminalShell->terminalProcessName.length == 0)
            ffStrbufAppendS(&result.error, "Terminal font needs successfull terminal detection");

        else if(!detectTerminalFontCommon(instance, terminalShell, &result))
            ffDetectTerminalFontPlatform(instance, terminalShell, &result);

        if(result.error.length == 0 && result.font.pretty.length == 0)
            ffStrbufAppendF(&result.error, "Unknown terminal: %s", terminalShell->terminalProcessName.chars);
    );
}
