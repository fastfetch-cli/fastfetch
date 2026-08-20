#include "fastfetch.h"
#include "terminalfont.h"
#include "detection/terminalshell/terminalshell.h"
#include "common/io.h"
#include "common/properties.h"

#ifdef FF_HAVE_FREETYPE
    #include "common/library.h"
    #include <ft2build.h>
    #include FT_FREETYPE_H
#endif

#define FF_TERMUX_FONT_PATH FASTFETCH_TARGET_DIR_HOME "/.termux/font.ttf"
#define FF_TERMUX_PREF_PATH "/data/data/com.termux/shared_prefs/com.termux_preferences.xml"

const char* detectTermux(FFTerminalFontResult* terminalFont) {
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();
    // SharedPreferences XML: <string name="fontsize">14</string>, in px
    if (ffParsePropFile(FF_TERMUX_PREF_PATH, "<string name=\"fontsize\">", &fontSize)) {
        ffStrbufAppendS(&fontSize, "px");
    }

    if (!ffPathExists(FF_TERMUX_FONT_PATH, FF_PATHTYPE_FILE)) {
        ffFontInitValues(&terminalFont->font, "monospace", fontSize.length ? fontSize.chars : nullptr);
        return nullptr;
    }

#ifdef FF_HAVE_FREETYPE

    FF_LIBRARY_LOAD_MESSAGE(freetype, "libfreetype" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(freetype, FT_Init_FreeType);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(freetype, FT_New_Face);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(freetype, FT_Done_Face);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(freetype, FT_Done_FreeType);

    FT_Library library = nullptr;
    FT_Face face = nullptr;
    const char* error = nullptr;

    if (ffFT_Init_FreeType(&library)) {
        error = "FT_Init_FreeType() failed";
        goto exit;
    }

    if (ffFT_New_Face(library, FF_TERMUX_FONT_PATH, 0, &face)) {
        error = "FT_NEW_Face(" FF_TERMUX_FONT_PATH ") failed";
        goto exit;
    }

    ffFontInitValues(&terminalFont->font, face->family_name, fontSize.length ? fontSize.chars : nullptr);

exit:
    if (face) {
        ffFT_Done_Face(face);
    }
    if (library) {
        ffFT_Done_FreeType(library);
    }

    return error;

#else

    ffFontInitValues(&terminalFont->font, "monospace", fontSize.length ? fontSize.chars : nullptr);
    return "Fastfetch was built without freetype2 support";

#endif
}

bool ffDetectTerminalFontPlatform(const FFTerminalResult* terminal, FFTerminalFontResult* terminalFont) {
    if (ffStrbufEqualS(&terminal->processName, "com.termux")) {
        ffStrbufSetS(&terminalFont->error, detectTermux(terminalFont));
    } else {
        bool ffDetectTerminalFontPlatformLinux(const FFTerminalResult* terminal, FFTerminalFontResult* terminalFont);
        return ffDetectTerminalFontPlatformLinux(terminal, terminalFont);
    }

    return true;
}
