#include "fastfetch.h"
#include "common/properties.h"
#include "common/thread.h"
#include "common/settings.h"
#include "detection/gtk_qt/gtk_qt.h"
#include "detection/displayserver/displayserver.h"

static inline bool allPropertiesSet(FFGTKResult* result) {
    return result->theme.length > 0 &&
        result->icons.length > 0 &&
        result->font.length > 0;
}

static inline void applyGTKSettings(FFGTKResult* result, const char* themeName, const char* iconsName, const char* fontName, const char* cursorTheme, int cursorSize, const char* wallpaper) {
    if (result->theme.length == 0) {
        ffStrbufAppendS(&result->theme, themeName);
    }

    if (result->icons.length == 0) {
        ffStrbufAppendS(&result->icons, iconsName);
    }

    if (result->font.length == 0) {
        ffStrbufAppendS(&result->font, fontName);
    }

    if (result->cursor.length == 0) {
        ffStrbufAppendS(&result->cursor, cursorTheme);
    }

    if (result->cursorSize.length == 0 && cursorSize > 0) {
        ffStrbufAppendF(&result->cursorSize, "%i", cursorSize);
    }

    if (result->wallpaper.length == 0) {
        ffStrbufAppendS(&result->wallpaper, wallpaper);
    }
}

static bool testXfconfWallpaperPropKey([[maybe_unused]] void* data, const char* key) {
    int count = 0;
    sscanf(key, "/backdrop/screen0/monitor%*[^/]/workspace0/last-image%n", &count);
    return count == 0;
}

static void detectGTKFromSettings(FFGTKResult* result) {
    static const char* themeName = nullptr;
    static const char* iconsName = nullptr;
    static const char* fontName = nullptr;
    static const char* cursorTheme = nullptr;
    static int cursorSize = 0;
    static const char* wallpaper = nullptr;

    static bool init = false;

    if (init) {
        applyGTKSettings(result, themeName, iconsName, fontName, cursorTheme, cursorSize, wallpaper);
        return;
    }

    init = true;

    const FFDisplayServerResult* wmde = ffConnectDisplayServer();

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
        ffEnlightenmentSettings settings = {};
        if (ffSettingsGetEnlightenmentProperty(&settings)) {
            themeName = settings.theme;
            iconsName = settings.icon_theme;
            fontName = settings.font;
            cursorTheme = settings.use_e_cursor ? "Enlightenment" : "Application";
            cursorSize = settings.cursor_size;
            wallpaper = settings.desktop_default_background;
        }
    }

    applyGTKSettings(result, themeName, iconsName, fontName, cursorTheme, cursorSize, wallpaper);
}

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
    detectGTKFromSettings(result);
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

#define FF_DETECT_GTK_IMPL(version)   \
    static FFGTKResult result;        \
    static bool init = false;         \
    if (init)                         \
        return &result;               \
    init = true;                      \
    ffStrbufInit(&result.theme);      \
    ffStrbufInit(&result.icons);      \
    ffStrbufInit(&result.font);       \
    ffStrbufInit(&result.cursor);     \
    ffStrbufInit(&result.cursorSize); \
    ffStrbufInit(&result.wallpaper);  \
    detectGTK(#version, &result);     \
    return &result;

const FFGTKResult* ffDetectGTK2(void) {
    FF_DETECT_GTK_IMPL(2)
}

const FFGTKResult* ffDetectGTK3(void) {
    FF_DETECT_GTK_IMPL(3)
}

const FFGTKResult* ffDetectGTK4(void) {
    FF_DETECT_GTK_IMPL(4)
}

#undef FF_CALCULATE_GTK_IMPL
