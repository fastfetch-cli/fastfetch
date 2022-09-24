#include "terminalfont.h"
#include "common/printing.h"
#include "common/font.h"
#include "common/io.h"
#include "common/library.h"
#include "detection/terminalshell.h"
#include "util/apple/osascript.h"

#include <stdlib.h>
#include <string.h>

#ifdef FF_HAVE_LIBPLIST
#include <plist/plist.h>

static const char* iterm2ParsePList(const FFinstance* instance, const FFstrbuf* content, const char* profile, FFTerminalFontResult* terminalFont)
{
    FF_LIBRARY_LOAD(libplist, &instance->config.libplist, "dlopen libplist failed", "libplist-2.0"FF_LIBRARY_EXTENSION, 2);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_is_binary);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_from_bin);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_from_xml);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_array_new_iter);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_access_path);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_array_next_item);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_string_val_compare);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_dict_get_item);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_get_string_ptr);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_free);

    plist_t root_node = NULL;
    if (ffplist_is_binary(content->chars, content->length))
        ffplist_from_bin(content->chars, content->length, &root_node);
    else
        ffplist_from_xml(content->chars, content->length, &root_node);

    const char* error = NULL;

    if(root_node == NULL)
    {
        error = "parsing iTerm preference file failed";
        goto exit;
    }

    plist_t bookmarks = ffplist_access_path(root_node, 1, "New Bookmarks");

    plist_array_iter iter = NULL;
    ffplist_array_new_iter(bookmarks, &iter);
    plist_t item = NULL;
    do
    {
        ffplist_array_next_item(bookmarks, iter, &item);
        if(ffplist_string_val_compare(ffplist_dict_get_item(item, "Name"), profile) == 0)
            break;
    } while(item != NULL);
    free(iter);

    if(item == NULL)
    {
        error = "find profile bookmark failed";
        goto exit;
    }

    item = ffplist_dict_get_item(item, "Normal Font");
    if (item == NULL)
    {
        error = "find Normal Font key failed";
        goto exit;
    }

    ffFontInitWithSpace(&terminalFont->font, ffplist_get_string_ptr(item, NULL));

exit:
    ffplist_free(root_node);
    dlclose(libplist);
    return error;
}

static void detectIterm2(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    const char* profile = getenv("ITERM_PROFILE");
    if (profile == NULL)
    {
        ffStrbufAppendS(&terminalFont->error, "environment variable ITERM_PROFILE not set");
        return;
    }

    FFstrbuf fileName;
    ffStrbufInit(&fileName);
    ffStrbufAppendS(&fileName, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&fileName, "/Library/Preferences/com.googlecode.iterm2.plist");

    FFstrbuf content;
    ffStrbufInit(&content);
    ffAppendFileBuffer(fileName.chars, &content);
    ffStrbufDestroy(&fileName);
    if(content.length == 0)
    {
        ffStrbufAppendS(&terminalFont->error, "reading iTerm preference file failed");
        ffStrbufDestroy(&content);
        return;
    }

    const char* error = iterm2ParsePList(instance, &content, profile, terminalFont);
    if(error != NULL)
        ffStrbufAppendS(&terminalFont->error, error);

    ffStrbufDestroy(&content);
}

#else
static void detectIterm2(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    FF_UNUSED(instance);
    ffStrbufAppendS(&terminalFont->error, "Fastfetch was compiled without libplist support");
}
#endif

static void detectAppleTerminal(FFTerminalFontResult* terminalFont)
{
    FFstrbuf fontName;
    ffStrbufInit(&fontName);
    ffOsascript("tell application \"Terminal\" to font name of window frontmost", &fontName);

    if(fontName.length == 0)
    {
        ffStrbufAppendS(&terminalFont->error, "executing osascript failed");
        ffStrbufDestroy(&fontName);
        return;
    }

    FFstrbuf fontSize;
    ffStrbufInit(&fontSize);
    ffOsascript("tell application \"Terminal\" to font size of window frontmost", &fontSize);

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&fontSize);
}

void ffDetectTerminalFontPlatform(const FFinstance* instance, const FFTerminalShellResult* terminalShell, FFTerminalFontResult* terminalFont)
{
    if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "iterm.app") == 0)
        detectIterm2(instance, terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "Apple_Terminal") == 0)
        detectAppleTerminal(terminalFont);
}
