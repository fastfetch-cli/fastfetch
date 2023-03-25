#include "terminalfont.h"
#include "common/properties.h"
#include "common/processing.h"
#include "detection/internal.h"
#include "detection/terminalshell/terminalshell.h"

static void detectAlacritty(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
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

FF_MAYBE_UNUSED static void detectTTY(FFTerminalFontResult* terminalFont)
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

#ifdef FF_HAVE_JSONC

#include "common/library.h"
#include "common/processing.h"

#include <stdlib.h>
#include <json-c/json.h>

typedef struct JSONCData
{
    FF_LIBRARY_SYMBOL(json_tokener_parse)
    FF_LIBRARY_SYMBOL(json_object_get_array)
    FF_LIBRARY_SYMBOL(json_object_is_type)
    FF_LIBRARY_SYMBOL(json_object_get_double)
    FF_LIBRARY_SYMBOL(json_object_get_string_len)
    FF_LIBRARY_SYMBOL(json_object_get_string)
    FF_LIBRARY_SYMBOL(json_object_object_get)
    FF_LIBRARY_SYMBOL(json_object_put)

    json_object* root;
} JSONCData;

static const char* detectWTProfile(JSONCData* data, json_object* profile, FFstrbuf* name, double* size)
{
    json_object* font = data->ffjson_object_object_get(profile, "font");
    if (!font)
        return "json_object_object_get(profile, \"font\"); failed";

    if (!data->ffjson_object_is_type(font, json_type_object))
        return "json_object_is_type(font, json_type_object) returns false";

    if (name->length == 0)
    {
        json_object* pface = data->ffjson_object_object_get(font, "face");
        if(data->ffjson_object_is_type(pface, json_type_string))
            ffStrbufAppendNS(name, (uint32_t) data->ffjson_object_get_string_len(pface), data->ffjson_object_get_string(pface));
    }

    if (*size < 0)
    {
        json_object* psize = data->ffjson_object_object_get(font, "size");
        if (data->ffjson_object_is_type(psize, json_type_int) || data->ffjson_object_is_type(psize, json_type_double))
            *size = data->ffjson_object_get_double(psize);
    }
    return NULL;
}

static inline void wrapJsoncFree(JSONCData* data)
{
    assert(data);
    if (data->root)
        data->ffjson_object_put(data->root);
}

static const char* detectFromWTImpl(const FFinstance* instance, FFstrbuf* content, FFstrbuf* name, double* size)
{
    FF_LIBRARY_LOAD(libjsonc, &instance->config.libJSONC, "dlopen libjson-c" FF_LIBRARY_EXTENSION" failed",
        #ifdef _WIN32
            "libjson-c-5" FF_LIBRARY_EXTENSION, -1
        #else
            "libjson-c" FF_LIBRARY_EXTENSION, 5
        #endif
    )
    JSONCData __attribute__((__cleanup__(wrapJsoncFree))) data = {};
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libjsonc, data, json_tokener_parse)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libjsonc, data, json_object_is_type)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libjsonc, data, json_object_get_array)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libjsonc, data, json_object_get_double)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libjsonc, data, json_object_get_string_len)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libjsonc, data, json_object_get_string)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libjsonc, data, json_object_object_get)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libjsonc, data, json_object_put)

    data.root = data.ffjson_tokener_parse(content->chars);
    if (!data.root)
        return "Failed to parse WT JSON config file";

    json_object* profiles = data.ffjson_object_object_get(data.root, "profiles");
    if (!profiles)
        return "json_object_object_get(root, \"profiles\") failed";

    FF_STRBUF_AUTO_DESTROY wtProfileId;
    ffStrbufInitS(&wtProfileId, getenv("WT_PROFILE_ID"));
    ffStrbufTrim(&wtProfileId, '\'');
    if (wtProfileId.length > 0)
    {
        array_list* list = data.ffjson_object_get_array(data.ffjson_object_object_get(profiles, "list"));
        if (list)
        {
            for (size_t idx = 0; idx < list->length; ++idx)
            {
                json_object* profile = (json_object*) list->array[idx];
                json_object* guid = data.ffjson_object_object_get(profile, "guid");

                if (!data.ffjson_object_is_type(guid, json_type_string))
                    continue;

                if(ffStrbufEqualS(&wtProfileId, data.ffjson_object_get_string(guid)))
                {
                    detectWTProfile(&data, profile, name, size);
                    break;
                }
            }
        }
    }

    json_object* defaults = data.ffjson_object_object_get(profiles, "defaults");
    if (defaults)
        detectWTProfile(&data, defaults, name, size);

    if(name->length == 0)
        ffStrbufSetS(name, "Cascadia Mono");
    if(*size < 0)
        *size = 12;
    return NULL;
}

#ifdef _WIN32
    #include "common/io/io.h"

    #include <shlobj.h>
#endif

static void detectFromWindowsTeriminal(const FFinstance* instance, const FFstrbuf* terminalExe, FFTerminalFontResult* terminalFont)
{
    //https://learn.microsoft.com/en-us/windows/terminal/install#settings-json-file
    FFstrbuf json;
    ffStrbufInit(&json);
    const char* error = NULL;

    #ifdef _WIN32
    if(terminalExe && terminalExe->length > 0 && !ffStrbufEqualS(terminalExe, "Windows Terminal"))
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
    double size = -1;
    error = detectFromWTImpl(instance, &json, &name, &size);
    ffStrbufDestroy(&json);

    if(error)
        ffStrbufAppendS(&terminalFont->error, error);
    else
    {
        char sizeStr[16];
        snprintf(sizeStr, sizeof(sizeStr), "%g", size);
        ffFontInitValues(&terminalFont->font, name.chars, sizeStr);
    }

    ffStrbufDestroy(&name);
}

#else //FF_HAVE_JSONC

static void detectFromWindowsTeriminal(const FFinstance* instance, const FFstrbuf* terminalExe, FFTerminalFontResult* terminalFont)
{
    FF_UNUSED(instance, terminalExe, terminalFont);
    ffStrbufAppendS(&terminalFont->error, "Fastfetch was built without json-c support");
}

#endif //FF_HAVE_JSONC

#endif //defined(_WIN32) || defined(__linux__)

FF_MAYBE_UNUSED static bool detectKitty(const FFinstance* instance, FFTerminalFontResult* result)
{
    FF_STRBUF_AUTO_DESTROY fontName;
    ffStrbufInit(&fontName);

    FF_STRBUF_AUTO_DESTROY fontSize;
    ffStrbufInit(&fontSize);

    FFpropquery fontQuery[] = {
        {"font_family ", &fontName},
        {"font_size ", &fontSize},
    };

    if(!ffParsePropFileConfigValues(instance, "kitty/kitty.conf", 2, fontQuery))
        return false;

    if(fontName.length == 0)
        ffStrbufSetS(&fontName, "monospace");
    if(fontSize.length == 0)
        ffStrbufSetS(&fontSize, "11.0");

    ffFontInitValues(&result->font, fontName.chars, fontSize.chars);

    return true;
}

static void detectTerminator(const FFinstance* instance, FFTerminalFontResult* result)
{
    FF_STRBUF_AUTO_DESTROY useSystemFont;
    ffStrbufInit(&useSystemFont);

    FF_STRBUF_AUTO_DESTROY fontName;
    ffStrbufInit(&fontName);

    FFpropquery fontQuery[] = {
        {"use_system_font =", &useSystemFont},
        {"font =", &fontName},
    };

    if(!ffParsePropFileConfigValues(instance, "terminator/config", 2, fontQuery))
    {
        ffStrbufAppendS(&result->error, "Couldn't read Terminator config file");
        return;
    }

    if(ffStrbufIgnCaseEqualS(&useSystemFont, "True"))
    {
        ffFontInitCopy(&result->font, "System");
        return;
    }

    if(fontName.length == 0)
        ffFontInitValues(&result->font, "mono", "8");
    else
        ffFontInitPango(&result->font, fontName.chars);
}

static bool detectWezterm(FF_MAYBE_UNUSED const FFinstance* instance, FFTerminalFontResult* result)
{
    FF_STRBUF_AUTO_DESTROY fontName;
    ffStrbufInit(&fontName);

    ffStrbufSetS(&result->error, ffProcessAppendStdOut(&fontName, (char* const[]){
        "wezterm",
        "ls-fonts",
        "--text",
        "a",
        NULL
    }));
    if(result->error.length)
        return false;

    //LeftToRight
    // 0 a    \u{61}       x_adv=7  cells=1  glyph=a,180  wezterm.font("JetBrains Mono", {weight="Regular", stretch="Normal", style="Normal"})
    //                                      <built-in>, BuiltIn
    ffStrbufSubstrAfterFirstC(&fontName, '"');
    ffStrbufSubstrBeforeFirstC(&fontName, '"');

    if(!fontName.length)
        return false;

    ffFontInitCopy(&result->font, fontName.chars);
    return true;
}

static bool detectTabby(FF_MAYBE_UNUSED const FFinstance* instance, FFTerminalFontResult* result)
{
    FF_STRBUF_AUTO_DESTROY fontName;
    ffStrbufInit(&fontName);

    FF_STRBUF_AUTO_DESTROY fontSize;
    ffStrbufInit(&fontSize);

    FFpropquery fontQuery[] = {
        {"font: ", &fontName},
        {"fontSize: ", &fontSize},
    };

    if(!ffParsePropFileConfigValues(instance, "tabby/config.yaml", 2, fontQuery))
        return false;

    if(fontName.length == 0)
        ffStrbufSetS(&fontName, "monospace");
    if(fontSize.length == 0)
        ffStrbufSetS(&fontSize, "14");

    ffFontInitValues(&result->font, fontName.chars, fontSize.chars);

    return true;
}

void ffDetectTerminalFontPlatform(const FFinstance* instance, const FFTerminalShellResult* terminalShell, FFTerminalFontResult* terminalFont);

static bool detectTerminalFontCommon(const FFinstance* instance, const FFTerminalShellResult* terminalShell, FFTerminalFontResult* terminalFont)
{
    if(ffStrbufStartsWithIgnCaseS(&terminalShell->terminalProcessName, "alacritty"))
        detectAlacritty(instance, terminalFont);
    else if(ffStrbufStartsWithIgnCaseS(&terminalShell->terminalProcessName, "terminator"))
        detectTerminator(instance, terminalFont);
    else if(ffStrbufStartsWithIgnCaseS(&terminalShell->terminalProcessName, "wezterm-gui"))
        detectWezterm(instance, terminalFont);
    else if(ffStrbufStartsWithIgnCaseS(&terminalShell->terminalProcessName, "tabby"))
        detectTabby(instance, terminalFont);

    #ifndef _WIN32
    else if(ffStrbufStartsWithIgnCaseS(&terminalShell->terminalExe, "/dev/pts/"))
        ffStrbufAppendS(&terminalFont->error, "Terminal font detection is not supported on PTS");
    else if(ffStrbufIgnCaseEqualS(&terminalShell->terminalProcessName, "kitty"))
        detectKitty(instance, terminalFont);
    else if(ffStrbufStartsWithIgnCaseS(&terminalShell->terminalExe, "/dev/tty"))
        detectTTY(terminalFont);
    #endif

    #if defined(_WIN32) || defined(__linux__)
    //Used by both Linux (WSL) and Windows
    else if(ffStrbufIgnCaseEqualS(&terminalShell->terminalProcessName, "Windows Terminal") ||
        ffStrbufIgnCaseEqualS(&terminalShell->terminalProcessName, "WindowsTerminal.exe"))
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
            ffStrbufAppendS(&result.error, "Terminal font needs successful terminal detection");

        else if(!detectTerminalFontCommon(instance, terminalShell, &result))
            ffDetectTerminalFontPlatform(instance, terminalShell, &result);

        if(result.error.length == 0 && result.font.pretty.length == 0)
            ffStrbufAppendF(&result.error, "Unknown terminal: %s", terminalShell->terminalProcessName.chars);
    );
}
