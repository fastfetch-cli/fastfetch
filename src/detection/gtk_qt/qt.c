#include "fastfetch.h"
#include "common/properties.h"
#include "common/thread.h"
#include "detection/gtk_qt/gtk_qt.h"
#include "detection/displayserver/displayserver.h"
#include "util/stringUtils.h"

#include <stdlib.h>
#include <string.h>

static inline bool allValuesSet(const FFQtResult* result)
{
    return
        result->widgetStyle.length > 0 &&
        result->colorScheme.length > 0 &&
        result->icons.length > 0 &&
        result->font.length > 0 &&
        result->wallpaper.length > 0;
}

typedef enum __attribute__((__packed__)) PlasmaCategory
{
    PLASMA_CATEGORY_GENERAL,
    PLASMA_CATEGORY_KDE,
    PLASMA_CATEGORY_ICONS,
    PLASMA_CATEGORY_OTHER
} PlasmaCategory;

static bool detectPlasmaFromFile(const char* filename, FFQtResult* result)
{
    FILE* kdeglobals = fopen(filename, "r");
    if(kdeglobals == NULL)
        return false;

    char* line = NULL;
    size_t len = 0;

    PlasmaCategory category = PLASMA_CATEGORY_OTHER;

    while(getline(&line, &len, kdeglobals) != -1)
    {
        if(line[0] == '[')
        {
            char categoryName[32];
            sscanf(line, "[%31[^]]", categoryName);

            if(ffStrEqualsIgnCase(categoryName, "General"))
                category = PLASMA_CATEGORY_GENERAL;
            else if(ffStrEqualsIgnCase(categoryName, "KDE"))
                category = PLASMA_CATEGORY_KDE;
            else if(ffStrEqualsIgnCase(categoryName, "Icons"))
                category = PLASMA_CATEGORY_ICONS;
            else
                category = PLASMA_CATEGORY_OTHER;

            continue;
        }

        if(category == PLASMA_CATEGORY_KDE && result->widgetStyle.length == 0)
            ffParsePropLine(line, "widgetStyle =", &result->widgetStyle);
        else if(category == PLASMA_CATEGORY_ICONS && result->icons.length == 0)
            ffParsePropLine(line, "Theme =", &result->icons);
        else if(category == PLASMA_CATEGORY_GENERAL)
        {
            if(result->colorScheme.length == 0)
                ffParsePropLine(line, "ColorScheme =", &result->colorScheme);

            if(result->font.length == 0)
                ffParsePropLine(line, "font =", &result->font);

            //Before plasma 5.23, "Font" was the key instead of "font". Since a lot of distros ship older versions, we test for both.
            if(result->font.length == 0)
                ffParsePropLine(line, "Font =", &result->font);
        }
    }

    free(line);

    fclose(kdeglobals);

    return true;
}

static void detectPlasma(FFQtResult* result)
{
    bool foundAFile = false;

    //We need to do this because we use multiple threads on configDirs
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateA(64);

    FF_LIST_FOR_EACH(FFstrbuf, configDir, instance.state.platform.configDirs)
    {
        ffStrbufSet(&baseDir, configDir);
        ffStrbufAppendS(&baseDir, "kdeglobals");

        if(detectPlasmaFromFile(baseDir.chars, result))
            foundAFile = true;

        ffStrbufSet(&baseDir, configDir);
        ffStrbufAppendS(&baseDir, "plasma-org.kde.plasma.desktop-appletsrc");

        ffParsePropFile(baseDir.chars, "Image=", &result->wallpaper);

        if(allValuesSet(result))
            return;
    }

    if(!foundAFile)
        return;

    //In Plasma the default value is never set in the config file, but the whole key-value is discarded.
    ///We must set these values by our self if the file exists (it always does here)
    if(result->widgetStyle.length == 0)
        ffStrbufAppendS(&result->widgetStyle, "Breeze");

    if(result->colorScheme.length == 0)
        ffStrbufAppendS(&result->colorScheme, "BreezeLight");

    if(result->icons.length == 0)
        ffStrbufAppendS(&result->icons, "Breeze");

    if(result->font.length == 0)
        ffStrbufAppendS(&result->font, "Noto Sans, 10");
}

static void detectLXQt(FFQtResult* result)
{
    ffParsePropFileConfigValues("lxqt/lxqt.conf", 3, (FFpropquery[]) {
        {"style = ", &result->widgetStyle},
        {"icon_theme = ", &result->icons},
        {"font = ", &result->font}
    });

    ffParsePropFileConfig("pcmanfm-qt/lxqt/settings.conf", "Wallpaper=", &result->wallpaper);
}

static void detectQtCt(char qver, FFQtResult* result)
{
    // qt5ct and qt6ct are technically separate applications, but they're both
    // by the same author and qt6ct understands qt5ct in qt6 applications as well.
    char file[] = "qtXct/qtXct.conf";
    file[2] = file[8] = qver;

    FF_STRBUF_AUTO_DESTROY font = ffStrbufCreate();

    ffParsePropFileConfigValues(file, 3, (FFpropquery[]) {
        {"style=", &result->widgetStyle},
        {"icon_theme=", &result->icons},
        {"general=", &font}
    });

    if (ffStrbufStartsWithC(&font, '@'))
    {
        // See QVariant notes on https://doc.qt.io/qt-5/qsettings.html and
        // https://github.com/fastfetch-cli/fastfetch/issues/1053#issuecomment-2197254769
        // Thankfully, newer versions use the more common font encoding.
        ffStrbufSetNS(&font, 5, file);
    }
    else if (qver == '5')
    {
        // #1864
        const char *p = font.chars;

        while (*p)
        {
            if (p[0] == '\\' && p[1] == 'x' && isxdigit(p[2]) && isxdigit(p[3]) && isxdigit(p[4]) && isxdigit(p[5]))
            {
                uint32_t codepoint = (uint32_t)strtoul((char[]) { p[2], p[3], p[4], p[5], '\0' }, NULL, 16);
                ffStrbufAppendUtf32CodePoint(&result->font, codepoint);
                p += 6;
            }
            else
            {
                ffStrbufAppendC(&result->font, *p++);
            }
        }
    }
    else
    {
        ffStrbufDestroy(&result->font);
        ffStrbufInitMove(&result->font, &font);
    }
}

static void detectKvantum(FFQtResult* result)
{
    ffParsePropFileConfigValues("Kvantum/kvantum.kvconfig", 1, (FFpropquery[]) {
        {"theme=", &result->widgetStyle},
    });
}

const FFQtResult* ffDetectQt(void)
{
    static FFQtResult result;

    static bool init = false;
    if(init)
        return &result;
    init = true;

    ffStrbufInit(&result.widgetStyle);
    ffStrbufInit(&result.colorScheme);
    ffStrbufInit(&result.icons);
    ffStrbufInit(&result.font);
    ffStrbufInit(&result.wallpaper);

    const FFDisplayServerResult* wmde = ffConnectDisplayServer();

    if(ffStrbufIgnCaseEqualS(&wmde->dePrettyName, FF_DE_PRETTY_PLASMA))
        detectPlasma(&result);
    else if(ffStrbufIgnCaseEqualS(&wmde->dePrettyName, FF_DE_PRETTY_LXQT))
        detectLXQt(&result);
    else
    {
        const char *qPlatformTheme = getenv("QT_QPA_PLATFORMTHEME");
        if(qPlatformTheme && (ffStrEquals(qPlatformTheme, "qt5ct") || ffStrEquals(qPlatformTheme, "qt6ct")))
            detectQtCt(qPlatformTheme[2], &result);
    }

    if(ffStrbufEqualS(&result.widgetStyle, "kvantum") || ffStrbufEqualS(&result.widgetStyle, "kvantum-dark"))
    {
        ffStrbufClear(&result.widgetStyle);
        detectKvantum(&result);
    }

    return &result;
}
