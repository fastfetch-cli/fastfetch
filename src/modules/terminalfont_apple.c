#include "fastfetch.h"
#include "common/printing.h"
#include "common/font.h"
#include "common/io.h"
#include "common/library.h"
#include "util/apple/osascript.h"
#include "terminalfont.h"

#include <stdlib.h>
#include <string.h>

#ifdef FF_HAVE_LIBPLIST

#include <plist/plist.h>

static char* printIterm2Impl(FFinstance* instance, const char* profile, FFstrbuf* content)
{
    FF_LIBRARY_LOAD(libplist, instance->config.libplist, "dlopen libplist failed", "libplist-2.0"FF_LIBRARY_EXTENSION, 2);
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

    if(root_node == NULL)
        return "parsing iTerm preference file failed";

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
        ffplist_free(root_node);
        return "find profile bookmark failed";
    }

    item = ffplist_dict_get_item(item, "Normal Font");
    if (item == NULL)
    {
        ffplist_free(root_node);
        return "find Normal Font key failed";
    }

    FFfont font;
    ffFontInitWithSpace(&font, ffplist_get_string_ptr(item, NULL));
    ffPrintTerminalFontResult(instance, font.name.chars, &font);
    ffFontDestroy(&font);
    ffplist_free(root_node);
    return NULL;
}

#endif //FF_HAVE_LIBPLIST

static void printIterm2(FFinstance* instance)
{
    #ifdef FF_HAVE_LIBPLIST
    const char* profile = getenv("ITERM_PROFILE");
    if (profile == NULL)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Unknown iTerm profile");
        return;
    }

    FFstrbuf fileName;
    ffStrbufInit(&fileName);
    ffStrbufAppendS(&fileName, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&fileName, "/Library/Preferences/com.googlecode.iterm2.plist");

    FFstrbuf content;
    ffStrbufInit(&content);
    bool success = ffAppendFileBuffer(fileName.chars, &content);
    ffStrbufDestroy(&fileName);

    if(success)
    {
        const char* error = printIterm2Impl(instance, profile, &content);
        if (error)
            ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "%s", error);
    }
    else
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Read iTerm preference file failed");

    ffStrbufDestroy(&content);
    #else
    ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Parse iTerm preferences requires libplist to be installed");
    #endif
}

static void printAppleTerminal(FFinstance* instance)
{
    FFstrbuf fontName;
    ffStrbufInit(&fontName);
    if(!ffOsascript("tell application \"Terminal\" to font name of window frontmost", &fontName))
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "osascript failed");
        return;
    }

    FFstrbuf fontSize;
    ffStrbufInit(&fontSize);
    ffOsascript("tell application \"Terminal\" to font size of window frontmost", &fontSize);

    FFfont font;
    ffFontInitValues(&font, fontName.chars, fontSize.chars);
    ffPrintTerminalFontResult(instance, fontName.chars, &font);
    ffFontDestroy(&font);

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&fontSize);
}

bool ffPrintTerminalFontPlatform(FFinstance* instance, const FFTerminalShellResult* shellInfo)
{
    bool success = true;
    if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "iterm.app") == 0)
        printIterm2(instance);
    else if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "Apple_Terminal") == 0)
        printAppleTerminal(instance);
    else
        success = false;
    return success;
}
