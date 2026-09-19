#include "common/library.h"
#include "common/io.h"
#include "common/path.h"
#include "common/processing.h"
#include "common/properties.h"
#include "common/windows/unicode.h"
#include "common/windows/registry.h"
#include "common/strutil.h"
#include "detection/terminalshell/terminalshell.h"
#include "terminalfont.h"

#include <shlobj.h>
#include <windows.h>
#include <stdlib.h>

// Windows Terminal resolves every setting through an inheritance chain. A single property,
// such as the font face or the font size, is resolved from the most to the least important
// source:
//   4. the active profile itself (settings.json -> profiles.list)
//   3. profiles.defaults (settings.json)
//   2. a JSON fragment updating the profile with "updates"
//   1. a JSON fragment defining the profile with "guid"
//   0. not set -> the builtin default reported below
// https://github.com/microsoft/terminal/blob/main/src/cascadia/TerminalSettingsModel/IInheritable.h
// https://github.com/microsoft/terminal/blob/main/src/cascadia/TerminalSettingsModel/CascadiaSettingsSerialization.cpp
enum {
    WT_FONT_PRIORITY_FRAGMENT = 1,
    WT_FONT_PRIORITY_FRAGMENT_UPDATES = 2,
    WT_FONT_PRIORITY_DEFAULTS = 3,
    WT_FONT_PRIORITY_PROFILE = 4,
};

typedef struct FFTerminalFontWT {
    FFstrbuf name;
    double size;
    uint8_t namePriority; // 0 if unset
    uint8_t sizePriority; // 0 if unset
} FFTerminalFontWT;

static inline void wrapWTFontFree(FFTerminalFontWT* result) {
    assert(result);
    ffStrbufDestroy(&result->name);
}

static FFTerminalFontWT ffTerminalFontWTCreate(void) {
    FFTerminalFontWT result = {
        .name = ffStrbufCreate(),
        .size = -1,
    };
    return result;
}

static void applyWTProfile(yyjson_val* profile, uint8_t priority, FFTerminalFontWT* result) {
    yyjson_val* font = yyjson_obj_get(profile, "font");
    if (!yyjson_is_obj(font)) {
        return;
    }

    if (result->namePriority < priority) {
        yyjson_val* face = yyjson_obj_get(font, "face");
        if (yyjson_is_str(face)) {
            ffStrbufClear(&result->name);
            ffStrbufAppendJsonVal(&result->name, face);
            if (result->name.length > 0) { // an empty face is treated as unset
                result->namePriority = priority;
            }
        }
    }

    if (result->sizePriority < priority) {
        yyjson_val* size = yyjson_obj_get(font, "size");
        if (yyjson_is_num(size)) {
            result->size = unsafe_yyjson_get_num(size);
            result->sizePriority = priority;
        }
    }
}

// Finds the profile matching `wtProfileId` in a `profiles` array.
// A fragment either defines a profile with "guid" or updates an existing one with "updates",
// the latter being more important, so it is looked up first.
// Note that "guid" and "updates" may be missing: yyjson_get_str() returns nullptr then.
static yyjson_val* findWTProfileInArray(yyjson_val* profiles, const FFstrbuf* wtProfileId, bool* fromUpdates) {
    if (!yyjson_is_arr(profiles)) {
        return nullptr;
    }

    for (uint8_t pass = 0; pass < 2; ++pass) {
        yyjson_val* profile;
        size_t idx, max;
        yyjson_arr_foreach (profiles, idx, max, profile) {
            const char* id = yyjson_get_str(yyjson_obj_get(profile, pass == 0 ? "updates" : "guid"));
            if (id && ffStrbufEqualS(wtProfileId, id)) {
                *fromUpdates = pass == 0;
                return profile;
            }
        }
    }

    return nullptr;
}

static inline void wrapYyjsonFree(yyjson_doc** doc) {
    assert(doc);
    if (*doc) {
        yyjson_doc_free(*doc);
    }
}

static const char* detectFromWTSettings(FFstrbuf* content, const FFstrbuf* wtProfileId, FFTerminalFontWT* result) {
    [[gnu::cleanup(wrapYyjsonFree)]] yyjson_doc* doc = yyjson_read_opts(content->chars, content->length, YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_TRAILING_COMMAS, nullptr, nullptr);
    if (!doc) {
        return "Failed to parse WT JSON config file";
    }

    yyjson_val* const root = yyjson_doc_get_root(doc);
    assert(root);

    yyjson_val* profiles = yyjson_obj_get(root, "profiles");
    if (!profiles) {
        return "yyjson_obj_get(root, \"profiles\") failed";
    }

    if (wtProfileId->length > 0) {
        bool fromUpdates = false;
        yyjson_val* profile = findWTProfileInArray(yyjson_obj_get(profiles, "list"), wtProfileId, &fromUpdates);
        if (profile) {
            applyWTProfile(profile, WT_FONT_PRIORITY_PROFILE, result);
        }
    }

    yyjson_val* defaults = yyjson_obj_get(profiles, "defaults");
    if (defaults) {
        applyWTProfile(defaults, WT_FONT_PRIORITY_DEFAULTS, result);
    }

    return nullptr;
}

// Windows Terminal reads fragment files from a two level directory layout and doesn't recurse:
//   <known folder>\Microsoft\Windows Terminal\Fragments\<app-name>\<file-name>.json
// https://learn.microsoft.com/en-us/windows/terminal/json-fragment-extensions#where-to-place-the-json-fragment-files
static void applyWTFragmentFile(const char* path, const FFstrbuf* wtProfileId, FFTerminalFontWT* result) {
    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    if (!ffReadFileBuffer(path, &content)) {
        return;
    }

    // Fragments are optional: an unreadable or malformed one must not fail font detection
    [[gnu::cleanup(wrapYyjsonFree)]] yyjson_doc* doc = yyjson_read_opts(content.chars, content.length, YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_TRAILING_COMMAS, nullptr, nullptr);
    if (!doc) {
        return;
    }

    yyjson_val* const root = yyjson_doc_get_root(doc);
    assert(root);

    bool fromUpdates = false;
    yyjson_val* profile = findWTProfileInArray(yyjson_obj_get(root, "profiles"), wtProfileId, &fromUpdates);
    if (profile) {
        applyWTProfile(profile, fromUpdates ? WT_FONT_PRIORITY_FRAGMENT_UPDATES : WT_FONT_PRIORITY_FRAGMENT, result);
    }
}

static void detectWTProfileFromFragmentsIn(const FFstrbuf* fragmentDir, const FFstrbuf* wtProfileId, FFTerminalFontWT* result) {
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateCopy(fragmentDir);
    const uint32_t baseLength = path.length;

    ffStrbufAppendC(&path, '*');
    WIN32_FIND_DATAA entry;
    FF_AUTO_CLOSE_DIR HANDLE hFind = FindFirstFileA(path.chars, &entry);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (!(entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || entry.cFileName[0] == '.') {
            continue;
        }

        ffStrbufSubstrBefore(&path, baseLength);
        ffStrbufAppendS(&path, entry.cFileName);
        ffStrbufAppendC(&path, '\\');
        const uint32_t appLength = path.length;

        ffStrbufAppendS(&path, "*.json");
        WIN32_FIND_DATAA fileEntry;
        FF_AUTO_CLOSE_DIR HANDLE hFile = FindFirstFileA(path.chars, &fileEntry);
        if (hFile != INVALID_HANDLE_VALUE) {
            do {
                if (fileEntry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    continue;
                }

                ffStrbufSubstrBefore(&path, appLength);
                ffStrbufAppendS(&path, fileEntry.cFileName);
                applyWTFragmentFile(path.chars, wtProfileId, result);
            } while (FindNextFileA(hFile, &fileEntry));
        }

        ffStrbufSubstrBefore(&path, baseLength);
    } while (FindNextFileA(hFind, &entry));
}

static void detectFromWTFragments(const FFstrbuf* wtProfileId, FFTerminalFontWT* result) {
    // Windows Terminal merges the user scoped fragments before the machine scoped ones,
    // so the user scoped fragments are more important
    static const KNOWNFOLDERID* const fragmentFolderIds[] = { &FOLDERID_LocalAppData, &FOLDERID_ProgramData };

    for (uint32_t i = 0; i < ARRAY_SIZE(fragmentFolderIds); ++i) {
        PWSTR folderW = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(fragmentFolderIds[i], KF_FLAG_DEFAULT, nullptr, &folderW))) {
            FF_STRBUF_AUTO_DESTROY fragmentDir = ffStrbufCreateWS(folderW);
            CoTaskMemFree(folderW);
            ffStrbufAppendS(&fragmentDir, "\\Microsoft\\Windows Terminal\\Fragments\\");

            if (ffPathExists(fragmentDir.chars, FF_PATHTYPE_DIRECTORY)) {
                detectWTProfileFromFragmentsIn(&fragmentDir, wtProfileId, result);
            }
        }
    }
}

static void detectFromWindowsTerminal(const FFstrbuf* terminalExe, FFTerminalFontResult* terminalFont) {
    // https://learn.microsoft.com/en-us/windows/terminal/install#settings-json-file
    FF_STRBUF_AUTO_DESTROY json = ffStrbufCreate();
    const char* error = nullptr;

    if (terminalExe && ffIsAbsolutePath(terminalExe->chars)) {
        FF_STRBUF_AUTO_DESTROY jsonPath = ffStrbufCreateA(MAX_PATH);
        ffStrbufAppendNS(&jsonPath, ffStrbufLastIndexC(terminalExe, '\\') + 1, terminalExe->chars);
        ffStrbufAppendS(&jsonPath, ".portable");

        if (ffPathExists(jsonPath.chars, FF_PATHTYPE_ANY)) {
            ffStrbufSubstrBefore(&jsonPath, jsonPath.length - strlen(".portable"));
            ffStrbufAppendS(&jsonPath, "settings\\settings.json");
            if (!ffAppendFileBuffer(jsonPath.chars, &json)) {
                error = "Error reading Windows Terminal portable settings JSON file";
            }
        } else {
            PWSTR localAppDataW = nullptr;
            if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, &localAppDataW))) {
                ffStrbufSetWS(&jsonPath, localAppDataW);
                CoTaskMemFree(localAppDataW);

                if (ffStrbufContainIgnCaseS(terminalExe, "_8wekyb3d8bbwe\\")) {
                    // Microsoft Store version
                    if (ffStrbufContainIgnCaseS(terminalExe, ".WindowsTerminalPreview_")) {
                        // Preview version
                        ffStrbufAppendS(&jsonPath, "\\Packages\\Microsoft.WindowsTerminalPreview_8wekyb3d8bbwe\\LocalState\\settings.json");
                        if (!ffAppendFileBuffer(jsonPath.chars, &json)) {
                            error = "Error reading Windows Terminal Preview settings JSON file";
                        }
                    } else {
                        // Stable version
                        ffStrbufAppendS(&jsonPath, "\\Packages\\Microsoft.WindowsTerminal_8wekyb3d8bbwe\\LocalState\\settings.json");
                        if (!ffAppendFileBuffer(jsonPath.chars, &json)) {
                            error = "Error reading Windows Terminal settings JSON file";
                        }
                    }
                } else {
                    ffStrbufAppendS(&jsonPath, "\\Microsoft\\Windows Terminal\\settings.json");
                    if (!ffAppendFileBuffer(jsonPath.chars, &json)) {
                        error = "Error reading Windows Terminal settings JSON file";
                    }
                }
            }
        }
    }

    if (!error && json.length == 0) {
        error = ffProcessAppendStdOut(&json, (char* const[]) { "cmd.exe", "/c",
                                                 // print the file content directly, so we don't need to handle the difference of Windows and POSIX path
                                                 "if exist %LOCALAPPDATA%\\Packages\\Microsoft.WindowsTerminal_8wekyb3d8bbwe\\LocalState\\settings.json "
                                                 "( type %LOCALAPPDATA%\\Packages\\Microsoft.WindowsTerminal_8wekyb3d8bbwe\\LocalState\\settings.json ) "
                                                 "else if exist %LOCALAPPDATA%\\Packages\\Microsoft.WindowsTerminalPreview_8wekyb3d8bbwe\\LocalState\\settings.json "
                                                 "( type %LOCALAPPDATA%\\Packages\\Microsoft.WindowsTerminalPreview_8wekyb3d8bbwe\\LocalState\\settings.json ) "
                                                 "else if exist \"%LOCALAPPDATA%\\Microsoft\\Windows Terminal\\settings.json\" "
                                                 "( type %LOCALAPPDATA%\\Microsoft\\Windows Terminal\\settings.json ) "
                                                 "else ( call )",
                                                 nullptr });
    }

    if (error) {
        ffStrbufAppendS(&terminalFont->error, error);
        return;
    }
    ffStrbufTrimRight(&json, '\n');
    if (json.length == 0) {
        ffStrbufAppendS(&terminalFont->error, "Cannot find file \"settings.json\"");
        return;
    }

    FF_STRBUF_AUTO_DESTROY wtProfileId = ffStrbufCreateS(getenv("WT_PROFILE_ID"));
    ffStrbufTrim(&wtProfileId, '\'');

    [[gnu::cleanup(wrapWTFontFree)]] FFTerminalFontWT result = ffTerminalFontWTCreate();

    error = detectFromWTSettings(&json, &wtProfileId, &result);
    if (error) {
        ffStrbufAppendS(&terminalFont->error, error);
        return;
    }

    // JSON fragments are only read when settings.json doesn't fully specify the font
    if (wtProfileId.length > 0 && (result.name.length == 0 || result.size < 0)) {
        detectFromWTFragments(&wtProfileId, &result);
    }

    if (result.name.length == 0) {
        ffStrbufAppendS(&result.name, "Cascadia Mono");
    }
    if (result.size < 0) {
        result.size = 12;
    }

    char sizeStr[16];
    snprintf(sizeStr, ARRAY_SIZE(sizeStr), "%g", result.size);
    ffFontInitValues(&terminalFont->font, result.name.chars, sizeStr);
}

static void detectMintty(FFTerminalFontResult* terminalFont) {
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    if (!ffParsePropFileConfigValues("mintty/config", 2, (FFpropquery[]) { { "Font=", &fontName }, { "FontHeight=", &fontSize } })) {
        ffParsePropFileConfigValues(".minttyrc", 2, (FFpropquery[]) { { "Font=", &fontName }, { "FontHeight=", &fontSize } });
    }
    if (fontName.length == 0) {
        ffStrbufAppendS(&fontName, "Lucida Console");
    }
    if (fontSize.length == 0) {
        ffStrbufAppendC(&fontSize, '9');
    }

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);
}

static void detectConhost(FFTerminalFontResult* terminalFont) {
    CONSOLE_FONT_INFOEX cfi = { .cbSize = sizeof(cfi) };
    if (!GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi)) {
        ffStrbufAppendS(&terminalFont->error, "GetCurrentConsoleFontEx() failed");
        return;
    }

    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreateWS(cfi.FaceName);

    char fontSize[16];
    _ultoa((unsigned long) (cfi.dwFontSize.Y), fontSize, 10);

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize);
}

static void detectConEmu(FFTerminalFontResult* terminalFont) {
    // https://conemu.github.io/en/ConEmuXml.html#search-sequence
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    const char* paths[] = { "ConEmuDir", "ConEmuBaseDir", "APPDATA" };
    for (uint32_t i = 0; i < ARRAY_SIZE(paths); ++i) {
        ffStrbufSetS(&path, getenv(paths[i]));
        if (path.length > 0) {
            ffStrbufAppendS(&path, "/ConEmu.xml");
            if (ffParsePropFileValues(path.chars, 2, (FFpropquery[]) { { "<value name=\"FontName\" type=\"string\" data=\"", &fontName }, { "<value name=\"FontSize\" type=\"ulong\" data=\"", &fontSize } })) {
                break;
            }
        }
    }

    if (fontName.length == 0 && fontSize.length == 0) {
        ffStrbufAppendS(&terminalFont->error, "Failed to parse ConEmu.xml");
        return;
    }

    if (fontName.length > 0) {
        ffStrbufSubstrBeforeLastC(&fontName, '"');
    } else {
        ffStrbufAppendS(&fontName, "Consola");
    }

    if (fontSize.length > 0) {
        ffStrbufSubstrBeforeLastC(&fontSize, '"');
    } else {
        ffStrbufAppendS(&fontSize, "14");
    }

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);
}

static void detectWarp(FFTerminalFontResult* terminalFont) {
    FF_AUTO_CLOSE_FD HANDLE key = nullptr;
    if (!ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"Software\\Warp.dev\\Warp", &key, &terminalFont->error)) {
        return;
    }

    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();
    if (ffRegReadValues(key, 2, (FFRegValueArg[]) { FF_ARG(fontName, L"FontName"), FF_ARG(fontSize, L"FontSize") }, &terminalFont->error)) {
        ffStrbufTrim(&fontName, '"');
        ffStrbufAppendS(&fontSize, "px");
    } else {
        ffStrbufSetS(&fontName, "Hack");
        ffStrbufSetS(&fontSize, "13.0px");
    }

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);

    FFstrbuf* fontWeight = FF_LIST_ADD(FFstrbuf, terminalFont->font.styles);
    ffStrbufInit(fontWeight);
    if (ffRegReadStrbuf(key, L"FontWeight", fontWeight, nullptr)) {
        ffStrbufTrim(fontWeight, '"');
    } else {
        ffStrbufSetStatic(fontWeight, "Normal");
    }
}

bool ffDetectTerminalFontPlatform(const FFTerminalResult* terminal, FFTerminalFontResult* terminalFont) {
    if (ffStrbufIgnCaseEqualS(&terminal->processName, "Windows Terminal") ||
        ffStrbufIgnCaseEqualS(&terminal->processName, "WindowsTerminal.exe")) {
        detectFromWindowsTerminal(&terminal->exe, terminalFont);
    } else if (ffStrbufIgnCaseEqualS(&terminal->processName, "mintty")) {
        detectMintty(terminalFont);
    } else if (ffStrbufIgnCaseEqualS(&terminal->processName, "conhost.exe")) {
        detectConhost(terminalFont);
    } else if (ffStrbufStartsWithIgnCaseS(&terminal->processName, "ConEmu")) {
        detectConEmu(terminalFont);
    } else if (ffStrbufStartsWithIgnCaseS(&terminal->processName, "warp")) {
        detectWarp(terminalFont);
    } else {
        return false;
    }
    return true;
}
