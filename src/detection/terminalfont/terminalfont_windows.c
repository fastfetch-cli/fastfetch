#include "common/library.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "common/properties.h"
#include "detection/terminalshell/terminalshell.h"
#include "util/windows/unicode.h"
#include "util/stringUtils.h"
#include "terminalfont.h"

#include <shlobj.h>
#include <windows.h>
#include <stdlib.h>

static const char* detectWTProfile(yyjson_val* profile, FFstrbuf* name, double* size)
{
    yyjson_val* font = yyjson_obj_get(profile, "font");
    if (!font)
        return "yyjson_obj_get(profile, \"font\"); failed";

    if (!yyjson_is_obj(font))
        return "yyjson_is_obj(font) returns false";

    if (name->length == 0)
    {
        yyjson_val* pface = yyjson_obj_get(font, "face");
        if(yyjson_is_str(pface))
            ffStrbufAppendS(name, unsafe_yyjson_get_str(pface));
    }

    if (*size < 0)
    {
        yyjson_val* psize = yyjson_obj_get(font, "size");
        if (yyjson_is_num(psize))
            *size = yyjson_get_num(psize);
    }
    return NULL;
}

static inline void wrapYyjsonFree(yyjson_doc** doc)
{
    assert(doc);
    if (*doc)
        yyjson_doc_free(*doc);
}

static const char* detectFromWTImpl(FFstrbuf* content, FFstrbuf* name, double* size)
{
    yyjson_doc* __attribute__((__cleanup__(wrapYyjsonFree))) doc = yyjson_read_opts(content->chars, content->length, YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_TRAILING_COMMAS | YYJSON_READ_ALLOW_INF_AND_NAN, NULL, NULL);
    if (!doc)
        return "Failed to parse WT JSON config file";

    yyjson_val* const root = yyjson_doc_get_root(doc);
    assert(root);

    yyjson_val* profiles = yyjson_obj_get(root, "profiles");
    if (!profiles)
        return "yyjson_obj_get(root, \"profiles\") failed";

    FF_STRBUF_AUTO_DESTROY wtProfileId = ffStrbufCreateS(getenv("WT_PROFILE_ID"));
    ffStrbufTrim(&wtProfileId, '\'');
    if (wtProfileId.length > 0)
    {
        yyjson_val* list = yyjson_obj_get(profiles, "list");
        if (yyjson_is_arr(list))
        {
            yyjson_val* profile;
            size_t idx, max;
            yyjson_arr_foreach(list, idx, max, profile)
            {
                yyjson_val* guid = yyjson_obj_get(profile, "guid");

                if(ffStrbufEqualS(&wtProfileId, yyjson_get_str(guid)))
                {
                    detectWTProfile(profile, name, size);
                    break;
                }
            }
        }
    }

    yyjson_val* defaults = yyjson_obj_get(profiles, "defaults");
    if (defaults)
        detectWTProfile(defaults, name, size);

    if(name->length == 0)
        ffStrbufSetS(name, "Cascadia Mono");
    if(*size < 0)
        *size = 12;
    return NULL;
}

static void detectFromWindowsTerminal(const FFstrbuf* terminalExe, FFTerminalFontResult* terminalFont)
{
    //https://learn.microsoft.com/en-us/windows/terminal/install#settings-json-file
    FF_STRBUF_AUTO_DESTROY json = ffStrbufCreate();
    const char* error = NULL;

    if(terminalExe && terminalExe->length > 0 && !ffStrbufEqualS(terminalExe, "Windows Terminal"))
    {
        char jsonPath[MAX_PATH + 1];
        char* pathEnd = ffStrCopyN(jsonPath, terminalExe->chars, ffStrbufLastIndexC(terminalExe, '\\') + 1);
        ffStrCopyN(pathEnd, ".portable", ARRAY_SIZE(jsonPath) - (size_t) (pathEnd - jsonPath) - 1);

        if(ffPathExists(jsonPath, FF_PATHTYPE_ANY))
        {
            ffStrCopyN(pathEnd, "settings\\settings.json", ARRAY_SIZE(jsonPath) - (size_t) (pathEnd - jsonPath) - 1);
            if(!ffAppendFileBuffer(jsonPath, &json))
                error = "Error reading Windows Terminal portable settings JSON file";
        }
        else if(SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, jsonPath)))
        {
            size_t remaining = ARRAY_SIZE(jsonPath) - strlen(jsonPath) - 1;
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
        return;
    }
    ffStrbufTrimRight(&json, '\n');
    if(json.length == 0)
    {
        ffStrbufAppendS(&terminalFont->error, "Cannot find file \"settings.json\"");
        return;
    }

    FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
    double size = -1;
    error = detectFromWTImpl(&json, &name, &size);

    if(error)
        ffStrbufAppendS(&terminalFont->error, error);
    else
    {
        char sizeStr[16];
        snprintf(sizeStr, ARRAY_SIZE(sizeStr), "%g", size);
        ffFontInitValues(&terminalFont->font, name.chars, sizeStr);
    }
}

static void detectMintty(FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    if(!ffParsePropFileConfigValues("mintty/config", 2, (FFpropquery[]) {
        {"Font=", &fontName},
        {"FontHeight=", &fontSize}
    }))
        ffParsePropFileConfigValues(".minttyrc", 2, (FFpropquery[]) {
            {"Font=", &fontName},
            {"FontHeight=", &fontSize}
        });
    if(fontName.length == 0)
        ffStrbufAppendS(&fontName, "Lucida Console");
    if(fontSize.length == 0)
        ffStrbufAppendC(&fontSize, '9');

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);
}

static void detectConhost(FFTerminalFontResult* terminalFont)
{
    CONSOLE_FONT_INFOEX cfi = { .cbSize = sizeof(cfi) };
    if(!GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi))
    {
        ffStrbufAppendS(&terminalFont->error, "GetCurrentConsoleFontEx() failed");
        return;
    }

    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreateWS(cfi.FaceName);

    char fontSize[16];
    _ultoa((unsigned long)(cfi.dwFontSize.Y), fontSize, 10);

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize);
}

static void detectConEmu(FFTerminalFontResult* terminalFont)
{
    //https://conemu.github.io/en/ConEmuXml.html#search-sequence
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    const char* paths[] = { "ConEmuDir", "ConEmuBaseDir", "APPDATA" };
    for (uint32_t i = 0; i < ARRAY_SIZE(paths); ++i)
    {
        ffStrbufSetS(&path, getenv(paths[i]));
        if(path.length > 0)
        {
            ffStrbufAppendS(&path, "/ConEmu.xml");
            if(ffParsePropFileValues(path.chars, 2, (FFpropquery[]){
                {"<value name=\"FontName\" type=\"string\" data=\"", &fontName},
                {"<value name=\"FontSize\" type=\"ulong\" data=\"", &fontSize}
            }))
                break;
        }
    }

    if(fontName.length == 0 && fontSize.length == 0)
    {
        ffStrbufAppendS(&terminalFont->error, "Failed to parse ConEmu.xml");
        return;
    }

    if(fontName.length > 0)
        ffStrbufSubstrBeforeLastC(&fontName, '"');
    else
        ffStrbufAppendS(&fontName, "Consola");

    if(fontSize.length > 0)
        ffStrbufSubstrBeforeLastC(&fontSize, '"');
    else
        ffStrbufAppendS(&fontSize, "14");

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);
}

void ffDetectTerminalFontPlatform(const FFTerminalResult* terminal, FFTerminalFontResult* terminalFont)
{
    if(ffStrbufIgnCaseEqualS(&terminal->processName, "Windows Terminal") ||
        ffStrbufIgnCaseEqualS(&terminal->processName, "WindowsTerminal.exe"))
        detectFromWindowsTerminal(&terminal->exe, terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "mintty"))
        detectMintty(terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "conhost.exe"))
        detectConhost(terminalFont);
    else if(ffStrbufStartsWithIgnCaseS(&terminal->processName, "ConEmu"))
        detectConEmu(terminalFont);
}
