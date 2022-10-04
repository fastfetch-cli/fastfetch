#include "fastfetch.h"
#include "detection/displayserver/displayserver.h"
#include "wmtheme.h"

#ifdef FF_HAVE_LIBPLIST
#include "common/library.h"
#include "common/io.h"
#include <plist/plist.h>

static const char* quartzCompositorParsePlist(FFinstance* instance, const FFstrbuf* content, FFstrbuf* theme)
{
    FF_LIBRARY_LOAD(libplist, &instance->config.libplist, "dlopen libplist failed", "libplist-2.0"FF_LIBRARY_EXTENSION, 2);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_is_binary);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_from_bin);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_from_xml);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_dict_get_item);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_get_string_ptr);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_get_uint_val);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libplist, plist_free);

    plist_t root_node = NULL;
    if (ffplist_is_binary(content->chars, content->length))
        ffplist_from_bin(content->chars, content->length, &root_node);
    else
        ffplist_from_xml(content->chars, content->length, &root_node);

    if(root_node == NULL)
        return "parsing Quartz Compositor preference file failed";

    plist_t pWmThemeColor = ffplist_dict_get_item(root_node, "AppleAccentColor");
    uint64_t wmThemeColor = 100;
    if (pWmThemeColor != NULL)
        ffplist_get_uint_val(pWmThemeColor, &wmThemeColor);
    switch (wmThemeColor)
    {
        case (uint64_t)-1: ffStrbufAppendS(theme, "Graphite"); break;
        case 0: ffStrbufAppendS(theme, "Red"); break;
        case 1: ffStrbufAppendS(theme, "Orange"); break;
        case 2: ffStrbufAppendS(theme, "Yellow"); break;
        case 3: ffStrbufAppendS(theme, "Green"); break;
        case 4: ffStrbufAppendS(theme, "Blue"); break;
        case 5: ffStrbufAppendS(theme, "Purple"); break;
        case 6: ffStrbufAppendS(theme, "Pink"); break;
        default: ffStrbufAppendS(theme, "Multicolor"); break;
    }

    plist_t pWmTheme = ffplist_dict_get_item(root_node, "AppleInterfaceStyle");
    ffStrbufAppendF(theme, " (%s)", pWmTheme != NULL ? ffplist_get_string_ptr(pWmTheme, NULL) : "Light");

    ffplist_free(root_node);
    return NULL;
}

static bool detectQuartzCompositor(FFinstance* instance, FFstrbuf* themeOrError)
{
    FFstrbuf fileName;
    ffStrbufInitS(&fileName, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&fileName, "/Library/Preferences/.GlobalPreferences.plist");

    FFstrbuf content;
    ffStrbufInit(&content);
    ffAppendFileBuffer(fileName.chars, &content);
    ffStrbufDestroy(&fileName);
    if(content.length == 0)
    {
        ffStrbufDestroy(&content);
        ffStrbufSetS(themeOrError, "reading Quartz Compositor preference file failed");
        return false;
    }

    const char* error = quartzCompositorParsePlist(instance, &content, themeOrError);
    ffStrbufDestroy(&content);
    if(error)
    {
        ffStrbufSetS(themeOrError, error);
        return false;
    }

    return true;
}
#else
static bool detectQuartzCompositor(FFinstance* instance, FFstrbuf* theme)
{
    FF_UNUSED(instance);
    ffStrbufSetS(theme, "Fastfetch was compiled without libplist support");
    return false;
}
#endif

bool ffDetectWmTheme(FFinstance* instance, FFstrbuf* themeOrError)
{
    const FFDisplayServerResult* wm = ffConnectDisplayServer(instance);

    if(wm->wmPrettyName.length == 0)
    {
        ffStrbufAppendS(themeOrError, "WM Theme needs sucessfull WM detection");
        return false;
    }

    if(ffStrbufIgnCaseCompS(&wm->wmPrettyName, "Quartz Compositor") == 0)
        return detectQuartzCompositor(instance, themeOrError);

    ffStrbufAppendS(themeOrError, "Unknown WM: ");
    ffStrbufAppend(themeOrError, &wm->dePrettyName);
    return false;
}
