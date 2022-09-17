#include "fastfetch.h"
#include "common/font.h"
#include "common/io.h"
#include "common/library.h"
#include "util/apple/osascript.h"
#include "terminalfont.h"

#include <stdlib.h>
#include <string.h>

#ifdef FF_HAVE_LIBPLIST

#include <plist/plist.h>

static const char* detectIterm2(FFinstance* instance, FFfont* font)
{
    const char* profile = getenv("ITERM_PROFILE");
    if (profile == NULL)
        return "Unknown iTerm profile";

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

    FFstrbuf fileName;
    ffStrbufInit(&fileName);
    ffStrbufAppendS(&fileName, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&fileName, "/Library/Preferences/com.googlecode.iterm2.plist");

    FFstrbuf content;
    ffStrbufInit(&content);
    bool success = ffAppendFileBuffer(fileName.chars, &content);
    ffStrbufDestroy(&fileName);

    if(!success)
    {
        dlclose(libplist);
        ffStrbufDestroy(&content);
        return "Read iTerm preference file failed";
    }

    plist_t root_node = NULL;
    if (ffplist_is_binary(content.chars, content.length))
        ffplist_from_bin(content.chars, content.length, &root_node);
    else
        ffplist_from_xml(content.chars, content.length, &root_node);
    ffStrbufDestroy(&content);

    if(root_node == NULL)
    {
        dlclose(libplist);
        return "parsing iTerm preference file failed";
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
        dlclose(libplist);
        ffplist_free(root_node);
        return "find profile bookmark failed";
    }

    item = ffplist_dict_get_item(item, "Normal Font");
    if (item == NULL)
    {
        dlclose(libplist);
        ffplist_free(root_node);
        return "find Normal Font key failed";
    }

    ffFontInitWithSpace(font, ffplist_get_string_ptr(item, NULL));
    ffplist_free(root_node);
    dlclose(libplist);
    return NULL;
}

#else //FF_HAVE_LIBPLIST

static const char* detectIterm2(FFinstance* instance, FFfont* font)
{
    FF_UNUSED(instance, font);
    return "Parse iTerm preferences requires libplist to be installed";
}

#endif //FF_HAVE_LIBPLIST

static const char* detectAppleTerminal(FFinstance* instance, FFfont* font)
{
    FF_UNUSED(instance);

    FFstrbuf fontName;
    ffStrbufInit(&fontName);
    if(!ffOsascript("tell application \"Terminal\" to font name of window frontmost", &fontName))
    {
        ffStrbufDestroy(&fontName);
        return "osascript failed";
    }

    FFstrbuf fontSize;
    ffStrbufInit(&fontSize);
    ffOsascript("tell application \"Terminal\" to font size of window frontmost", &fontSize);

    ffFontInitValues(font, fontName.chars, fontSize.chars);
    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&fontSize);
    return NULL;
}

const char* ffDetectTerminalFontPlatform(FFinstance* instance, const FFTerminalShellResult* shellInfo, FFfont* font)
{
    if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "iterm.app") == 0)
        return detectIterm2(instance, font);

    if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "Apple_Terminal") == 0)
        return detectAppleTerminal(instance, font);

    return "";
}
