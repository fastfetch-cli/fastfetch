#include "fastfetch.h"
#include "detection/qt.h"
#include "common/properties.h"
#include "detection/displayserver/displayserver.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static inline bool allValuesSet(const FFQtResult* result)
{
    return
        result->widgetStyle.length > 0 &&
        result->colorScheme.length > 0 &&
        result->icons.length > 0 &&
        result->font.length > 0;
}

typedef enum PlasmaCategory
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

            if(strcasecmp(categoryName, "General") == 0)
                category = PLASMA_CATEGORY_GENERAL;
            else if(strcasecmp(categoryName, "KDE") == 0)
                category = PLASMA_CATEGORY_KDE;
            else if(strcasecmp(categoryName, "Icons") == 0)
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

    if(line != NULL)
        free(line);

    fclose(kdeglobals);

    return true;
}

static void detectPlasma(const FFinstance* instance, FFQtResult* result)
{
    bool foundAFile = false;

    //We need to do this because we use multiple threads on configDirs
    FFstrbuf baseDir;
    ffStrbufInitA(&baseDir, 64);

    for(uint32_t i = 0; i < instance->state.configDirs.length; i++)
    {
        ffStrbufSet(&baseDir, ffListGet(&instance->state.configDirs, i));
        ffStrbufAppendS(&baseDir, "kdeglobals");

        if(detectPlasmaFromFile(baseDir.chars, result))
            foundAFile = true;

        if(allValuesSet(result))
        {
            ffStrbufDestroy(&baseDir);
            return;
        }
    }

    ffStrbufDestroy(&baseDir);
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

static void detectLXQt(const FFinstance* instance, FFQtResult* result)
{
    ffParsePropFileConfigValues(instance, "lxqt/lxqt.conf", 3, (FFpropquery[]) {
        {"style = ", &result->widgetStyle},
        {"icon_theme = ", &result->icons},
        {"font = ", &result->font}
    });
}

const FFQtResult* ffDetectQt(const FFinstance* instance)
{
    static FFQtResult result;

    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;
    pthread_mutex_lock(&mutex);
    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }
    init = true;

    ffStrbufInit(&result.widgetStyle);
    ffStrbufInit(&result.colorScheme);
    ffStrbufInit(&result.icons);
    ffStrbufInit(&result.font);

    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, "KDE Plasma") == 0)
        detectPlasma(instance, &result);
    else if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, "LXQt") == 0)
        detectLXQt(instance, &result);

    pthread_mutex_unlock(&mutex);
    return &result;
}
