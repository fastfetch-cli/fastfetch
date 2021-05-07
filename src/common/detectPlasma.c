#include "fastfetch.h"

#include <string.h>
#include <pthread.h>

typedef enum PlasmaCategory
{
    PLASMA_CATEGORY_GENERAL,
    PLASMA_CATEGORY_KDE,
    PLASMA_CATEGORY_ICONS,
    PLASMA_CATEGORY_OTHER
} PlasmaCategory;

static bool detectFromConfigFile(const FFstrbuf* filename, FFPlasmaResult* result)
{
    FILE* kdeglobals = fopen(filename->chars, "r");
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
            sscanf(line, "[%[^]]", categoryName);

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

        if(category == PLASMA_CATEGORY_KDE)
        {
            sscanf(line, "widgetStyle=%[^\n]", result->widgetStyle.chars);
            sscanf(line, "widgetStyle=\"%[^\"]+", result->widgetStyle.chars);
        }
        else if(category == PLASMA_CATEGORY_GENERAL)
        {
            sscanf(line, "ColorScheme=%[^\n]", result->colorScheme.chars);
            sscanf(line, "ColorScheme=\"%[^\"]+", result->colorScheme.chars);

            sscanf(line, "font=%[^\n]", result->font.chars);
            sscanf(line, "font=\"%[^\"]+", result->font.chars);
        }
        else if(category == PLASMA_CATEGORY_ICONS)
        {
            sscanf(line, "Theme=%[^\n]", result->icons.chars);
            sscanf(line, "Theme=\"%[^\"]+", result->icons.chars);
        }
    }

    ffStrbufRecalculateLength(&result->widgetStyle);
    ffStrbufRecalculateLength(&result->colorScheme);
    ffStrbufRecalculateLength(&result->icons);
    ffStrbufRecalculateLength(&result->font);

    if(line != NULL)
        free(line);

    fclose(kdeglobals);

    return true;
}

const FFPlasmaResult* ffDetectPlasma(FFinstance* instance)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static FFPlasmaResult result;
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

    bool foundAFile = false;

    //We need to do this because we use multiple threads on configDirs
    FFstrbuf baseDirCopy;
    ffStrbufInitA(&baseDirCopy, 64);

    for(uint32_t i = 0; i < instance->state.configDirs.length; i++)
    {
        FFstrbuf* baseDir = (FFstrbuf*) ffListGet(&instance->state.configDirs, i);

        ffStrbufSet(&baseDirCopy, baseDir);
        ffStrbufAppendS(&baseDirCopy, "/kdeglobals");

        if(detectFromConfigFile(&baseDirCopy, &result))
            foundAFile = true;

        if(
            result.widgetStyle.length > 0 &&
            result.colorScheme.length > 0 &&
            result.icons.length > 0 &&
            result.font.length > 0
        ) break;
    }

    ffStrbufDestroy(&baseDirCopy);

    if(!foundAFile)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }

    //In Plasma the default value is never set in the config file, but the whole key-value is discarded.
    ///We must set these values by our self if the file exists (it always does here)
    if(result.widgetStyle.length == 0)
        ffStrbufAppendS(&result.widgetStyle, "Breeze");

    if(result.colorScheme.length == 0)
        ffStrbufAppendS(&result.colorScheme, "BreezeLight");

    if(result.icons.length == 0)
        ffStrbufAppendS(&result.icons, "Breeze");

    if(result.font.length == 0)
        ffStrbufAppendS(&result.font, "Noto Sans, 10");

    pthread_mutex_unlock(&mutex);
    return &result;
}
