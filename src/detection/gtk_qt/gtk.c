#include "fastfetch.h"
#include "common/properties.h"
#include "common/settings.h"
#include "common/FFcache.h"
#include "detection/gtk_qt/gtk_qt.h"
#include "detection/displayserver/displayserver.h"

static inline bool allPropertiesSet(FFGTKResult* result) {
    return result->theme.length > 0 &&
        result->icons.length > 0 &&
        result->font.length > 0;
}

// The values read from the desktop settings daemons, shared by the GTK2/3/4 results. They are
// owned copies on purpose: `settings.c` hands out strings it leaks deliberately (see the
// "Leaks value.chars" notes there), so borrowing them past a `--dynamic-interval` round would
// leak one set per round. Copying them also lets `destroy` release them.
typedef struct FFGTKSettings {
    FFstrbuf theme;
    FFstrbuf icons;
    FFstrbuf font;
    FFstrbuf cursor;
    FFstrbuf wallpaper;
    int32_t cursorSize;
} FFGTKSettings;

static inline void applyGTKSettings(FFGTKResult* result, const FFGTKSettings* settings) {
    if (result->theme.length == 0) {
        ffStrbufAppend(&result->theme, &settings->theme);
    }

    if (result->icons.length == 0) {
        ffStrbufAppend(&result->icons, &settings->icons);
    }

    if (result->font.length == 0) {
        ffStrbufAppend(&result->font, &settings->font);
    }

    if (result->cursor.length == 0) {
        ffStrbufAppend(&result->cursor, &settings->cursor);
    }

    if (result->cursorSize.length == 0 && settings->cursorSize > 0) {
        ffStrbufAppendSInt(&result->cursorSize, settings->cursorSize);
    }

    if (result->wallpaper.length == 0) {
        ffStrbufAppend(&result->wallpaper, &settings->wallpaper);
    }
}

static bool testXfconfWallpaperPropKey([[maybe_unused]] void* data, const char* key) {
    int count = 0;
    sscanf(key, "/backdrop/screen0/monitor%*[^/]/workspace0/last-image%n", &count);
    return count == 0;
}

static FFGTKSettings gtkSettings;

static void initGTKSettings(void* storage) {
    FFGTKSettings* settings = storage;

    ffStrbufInit(&settings->theme);
    ffStrbufInit(&settings->icons);
    ffStrbufInit(&settings->font);
    ffStrbufInit(&settings->cursor);
    ffStrbufInit(&settings->wallpaper);

    const char* themeName = nullptr;
    const char* iconsName = nullptr;
    const char* fontName = nullptr;
    const char* cursorTheme = nullptr;
    int cursorSize = 0;
    const char* wallpaper = nullptr;

    const FFDisplayServerResult* wmde = ffConnectDisplayServer();

    if (wmde->dePrettyName.length > 0) {
        if (ffStrbufIgnCaseEqualS(&wmde->dePrettyName, FF_DE_PRETTY_XFCE4)) {
            themeName = ffSettingsGetXFConf("xsettings", "/Net/ThemeName", FF_VARIANT_TYPE_STRING).strValue;
            iconsName = ffSettingsGetXFConf("xsettings", "/Net/IconThemeName", FF_VARIANT_TYPE_STRING).strValue;
            fontName = ffSettingsGetXFConf("xsettings", "/Gtk/FontName", FF_VARIANT_TYPE_STRING).strValue;
            cursorTheme = ffSettingsGetXFConf("xsettings", "/Gtk/CursorThemeName", FF_VARIANT_TYPE_STRING).strValue;
            cursorSize = ffSettingsGetXFConf("xsettings", "/Gtk/CursorThemeSize", FF_VARIANT_TYPE_INT).intValue;
            wallpaper = ffSettingsGetXFConfFirstMatch("xfce4-desktop", "/backdrop/screen0", FF_VARIANT_TYPE_STRING, nullptr, testXfconfWallpaperPropKey).strValue;
        } else if (ffStrbufIgnCaseEqualS(&wmde->dePrettyName, FF_DE_PRETTY_CINNAMON)) {
            themeName = ffSettingsGetGnome("/org/cinnamon/desktop/interface/gtk-theme", "org.cinnamon.desktop.interface", nullptr, "gtk-theme", FF_VARIANT_TYPE_STRING).strValue;
            iconsName = ffSettingsGetGnome("/org/cinnamon/desktop/interface/icon-theme", "org.cinnamon.desktop.interface", nullptr, "icon-theme", FF_VARIANT_TYPE_STRING).strValue;
            fontName = ffSettingsGetGnome("/org/cinnamon/desktop/interface/font-name", "org.cinnamon.desktop.interface", nullptr, "font-name", FF_VARIANT_TYPE_STRING).strValue;
            cursorTheme = ffSettingsGetGnome("/org/cinnamon/desktop/interface/cursor-theme", "org.cinnamon.desktop.interface", nullptr, "cursor-theme", FF_VARIANT_TYPE_STRING).strValue;
            cursorSize = ffSettingsGetGnome("/org/cinnamon/desktop/interface/cursor-size", "org.cinnamon.desktop.interface", nullptr, "cursor-size", FF_VARIANT_TYPE_INT).intValue;
            wallpaper = ffSettingsGetGnome("/org/cinnamon/desktop/background/picture-uri", "org.cinnamon.desktop.background", nullptr, "picture-uri", FF_VARIANT_TYPE_STRING).strValue;
        } else if (ffStrbufIgnCaseEqualS(&wmde->dePrettyName, FF_DE_PRETTY_MATE)) {
            themeName = ffSettingsGetGnome("/org/mate/interface/gtk-theme", "org.mate.interface", nullptr, "gtk-theme", FF_VARIANT_TYPE_STRING).strValue;
            iconsName = ffSettingsGetGnome("/org/mate/interface/icon-theme", "org.mate.interface", nullptr, "icon-theme", FF_VARIANT_TYPE_STRING).strValue;
            fontName = ffSettingsGetGnome("/org/mate/interface/font-name", "org.mate.interface", nullptr, "font-name", FF_VARIANT_TYPE_STRING).strValue;
            cursorTheme = ffSettingsGetGnome("/org/mate/peripherals-mouse/cursor-theme", "org.mate.peripherals-mouse", nullptr, "cursor-theme", FF_VARIANT_TYPE_STRING).strValue;
            cursorSize = ffSettingsGetGnome("/org/mate/peripherals-mouse/cursor-size", "org.mate.peripherals-mouse", nullptr, "cursor-size", FF_VARIANT_TYPE_INT).intValue;
            wallpaper = ffSettingsGetGnome("/org/mate/desktop/background", "org.mate.background", nullptr, "picture-filename", FF_VARIANT_TYPE_STRING).strValue;
        } else if (
            ffStrbufIgnCaseEqualS(&wmde->dePrettyName, FF_DE_PRETTY_GNOME) ||
            ffStrbufIgnCaseEqualS(&wmde->dePrettyName, FF_DE_PRETTY_GNOME_CLASSIC) ||
            ffStrbufIgnCaseEqualS(&wmde->dePrettyName, FF_DE_PRETTY_UNITY) ||
            ffStrbufIgnCaseEqualS(&wmde->dePrettyName, FF_DE_PRETTY_BUDGIE) ||
            ffStrbufIgnCaseEqualS(&wmde->dePrettyName, FF_DE_PRETTY_NEBIDE)) {
            themeName = ffSettingsGetGnome("/org/gnome/desktop/interface/gtk-theme", "org.gnome.desktop.interface", nullptr, "gtk-theme", FF_VARIANT_TYPE_STRING).strValue;
            iconsName = ffSettingsGetGnome("/org/gnome/desktop/interface/icon-theme", "org.gnome.desktop.interface", nullptr, "icon-theme", FF_VARIANT_TYPE_STRING).strValue;
            fontName = ffSettingsGetGnome("/org/gnome/desktop/interface/font-name", "org.gnome.desktop.interface", nullptr, "font-name", FF_VARIANT_TYPE_STRING).strValue;
            cursorTheme = ffSettingsGetGnome("/org/gnome/desktop/interface/cursor-theme", "org.gnome.desktop.interface", nullptr, "cursor-theme", FF_VARIANT_TYPE_STRING).strValue;
            cursorSize = ffSettingsGetGnome("/org/gnome/desktop/interface/cursor-size", "org.gnome.desktop.interface", nullptr, "cursor-size", FF_VARIANT_TYPE_INT).intValue;
            wallpaper = ffSettingsGetGnome("/org/gnome/desktop/background/picture-uri", "org.gnome.desktop.background", nullptr, "picture-uri", FF_VARIANT_TYPE_STRING).strValue;
        } else if (
            ffStrbufIgnCaseEqualS(&wmde->dePrettyName, FF_DE_PRETTY_ENLIGHTENMENT)) {
            ffEnlightenmentSettings enlightenmentSettings = {};
            if (ffSettingsGetEnlightenmentProperty(&enlightenmentSettings)) {
                themeName = enlightenmentSettings.theme;
                iconsName = enlightenmentSettings.icon_theme;
                fontName = enlightenmentSettings.font;
                cursorTheme = enlightenmentSettings.use_e_cursor ? "Enlightenment" : "Application";
                cursorSize = enlightenmentSettings.cursor_size;
                wallpaper = enlightenmentSettings.desktop_default_background;
            }
        }
    } else {
        // Standalone WMs (Hyprland, sway, niri, i3, ...) report no DE and have no settings
        // daemon, but GTK apps on them still read org.gnome.desktop.interface.
        // Read via DConf, not GSettings: GSettings synthesizes the schema default for keys
        // the user never set, DConf returns nothing. Runs last; applyGTKSettings() only
        // fills still-empty fields, so config files always win.
        themeName = ffSettingsGetDConf("/org/gnome/desktop/interface/gtk-theme", FF_VARIANT_TYPE_STRING).strValue;
        iconsName = ffSettingsGetDConf("/org/gnome/desktop/interface/icon-theme", FF_VARIANT_TYPE_STRING).strValue;
        fontName = ffSettingsGetDConf("/org/gnome/desktop/interface/font-name", FF_VARIANT_TYPE_STRING).strValue;
        cursorTheme = ffSettingsGetDConf("/org/gnome/desktop/interface/cursor-theme", FF_VARIANT_TYPE_STRING).strValue;
        cursorSize = ffSettingsGetDConf("/org/gnome/desktop/interface/cursor-size", FF_VARIANT_TYPE_INT).intValue;
    }

    // The strings above are owned by `settings.c`, which leaks them on purpose, so copy them into
    // the cache instead of borrowing them: this runs again on every `--dynamic-interval` round.
    ffStrbufSetS(&settings->theme, themeName);
    ffStrbufSetS(&settings->icons, iconsName);
    ffStrbufSetS(&settings->font, fontName);
    ffStrbufSetS(&settings->cursor, cursorTheme);
    ffStrbufSetS(&settings->wallpaper, wallpaper);
    settings->cursorSize = cursorSize;
}

static void destroyGTKSettings(void* storage) {
    FFGTKSettings* settings = storage;

    ffStrbufDestroy(&settings->theme);
    ffStrbufDestroy(&settings->icons);
    ffStrbufDestroy(&settings->font);
    ffStrbufDestroy(&settings->cursor);
    ffStrbufDestroy(&settings->wallpaper);
    settings->cursorSize = 0;
}

static FFcacheEntry ffCacheEntryGTKSettings = {
    .name = "gtk-settings",
    .storage = &gtkSettings,
    .init = initGTKSettings,
    .destroy = destroyGTKSettings,
};

static void detectGTKFromConfigFile(const char* filename, FFGTKResult* result) {
    ffParsePropFileValues(filename, 5, (FFpropquery[]) { { "gtk-theme-name =", &result->theme }, { "gtk-icon-theme-name =", &result->icons }, { "gtk-font-name =", &result->font }, { "gtk-cursor-theme-name =", &result->cursor }, { "gtk-cursor-theme-size =", &result->cursorSize } });
}

static void detectGTKFromConfigDir(FFstrbuf* configDir, const char* version, FFGTKResult* result) {
    uint32_t configDirLength = configDir->length;

    // <configdir>/gtk-<version>.0/settings.ini
    ffStrbufAppendS(configDir, "gtk-");
    ffStrbufAppendS(configDir, version);
    ffStrbufAppendS(configDir, ".0/settings.ini");
    detectGTKFromConfigFile(configDir->chars, result);
    ffStrbufSubstrBefore(configDir, configDirLength);
    if (allPropertiesSet(result)) {
        return;
    }

    // <configdir>/gtk-<version>.0/gtkrc
    ffStrbufAppendS(configDir, "gtk-");
    ffStrbufAppendS(configDir, version);
    ffStrbufAppendS(configDir, ".0/gtkrc");
    detectGTKFromConfigFile(configDir->chars, result);
    ffStrbufSubstrBefore(configDir, configDirLength);
    if (allPropertiesSet(result)) {
        return;
    }

    // <configdir>/gtkrc-<version>.0
    ffStrbufAppendS(configDir, "gtkrc-");
    ffStrbufAppendS(configDir, version);
    ffStrbufAppendS(configDir, ".0");
    detectGTKFromConfigFile(configDir->chars, result);
    ffStrbufSubstrBefore(configDir, configDirLength);
    if (allPropertiesSet(result)) {
        return;
    }

    // <configdir>/.gtkrc-<version>.0
    ffStrbufAppendS(configDir, ".gtkrc-");
    ffStrbufAppendS(configDir, version);
    ffStrbufAppendS(configDir, ".0");
    detectGTKFromConfigFile(configDir->chars, result);
    ffStrbufSubstrBefore(configDir, configDirLength);
}

static void detectGTK(const char* version, FFGTKResult* result) {
    // Mate, Cinnamon, GNOME, Unity, Budgie use dconf to save theme config
    // On other DEs, this will do nothing
    applyGTKSettings(result, ffCacheGet(&ffCacheEntryGTKSettings));
    if (allPropertiesSet(result)) {
        return;
    }

    // We need to do this because we use multiple threads on configDirs
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateA(64);

    FF_LIST_FOR_EACH (FFstrbuf, configDir, instance.state.platform.configDirs) {
        ffStrbufSet(&baseDir, configDir);
        detectGTKFromConfigDir(&baseDir, version, result);
        if (allPropertiesSet(result)) {
            break;
        }
    }
}

static void initGTKResult(const char* version, FFGTKResult* result) {
    ffStrbufInit(&result->theme);
    ffStrbufInit(&result->icons);
    ffStrbufInit(&result->font);
    ffStrbufInit(&result->cursor);
    ffStrbufInit(&result->cursorSize);
    ffStrbufInit(&result->wallpaper);
    detectGTK(version, result);
}

static void destroyGTKResult(void* storage) {
    FFGTKResult* result = storage;

    ffStrbufDestroy(&result->theme);
    ffStrbufDestroy(&result->icons);
    ffStrbufDestroy(&result->font);
    ffStrbufDestroy(&result->cursor);
    ffStrbufDestroy(&result->cursorSize);
    ffStrbufDestroy(&result->wallpaper);
}

static void initGTK2Result(void* storage) {
    initGTKResult("2", storage);
}

static void initGTK3Result(void* storage) {
    initGTKResult("3", storage);
}

static void initGTK4Result(void* storage) {
    initGTKResult("4", storage);
}

// Each version gets its own result and its own entry, so a refresh drops all three (each one is
// then rebuilt on demand, in the same lazy order as a single-shot run). They share the single
// `gtk-settings` entry above, which keeps a round at one settings query instead of three.
#define FF_DETECT_GTK_IMPL(version)       \
    static FFGTKResult result;            \
    static FFcacheEntry entry = {         \
        .name = "gtk" #version,           \
        .storage = &result,               \
        .init = initGTK##version##Result, \
        .destroy = destroyGTKResult,      \
    };                                    \
    return ffCacheGet(&entry);

const FFGTKResult* ffDetectGTK2(void) {
    FF_DETECT_GTK_IMPL(2)
}

const FFGTKResult* ffDetectGTK3(void) {
    FF_DETECT_GTK_IMPL(3)
}

const FFGTKResult* ffDetectGTK4(void) {
    FF_DETECT_GTK_IMPL(4)
}

#undef FF_DETECT_GTK_IMPL
