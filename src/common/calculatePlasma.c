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

void ffCalculatePlasma(FFinstance* instance)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return;
    }

    init = true;

    ffStrbufInit(&instance->state.plasma.widgetStyle);
    ffStrbufInit(&instance->state.plasma.colorScheme);
    ffStrbufInit(&instance->state.plasma.icons);
    ffStrbufInit(&instance->state.plasma.font);

    FF_STRBUF_CREATE(kdeglobalsFile);
    ffStrbufAppendS(&kdeglobalsFile, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&kdeglobalsFile, "/.config/kdeglobals");

    FILE* kdeglobals = fopen(kdeglobalsFile.chars, "r");
    if(kdeglobals == NULL)
    {
        ffStrbufDestroy(&kdeglobalsFile);
        pthread_mutex_unlock(&mutex);
        return;
    }

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
            sscanf(line, "widgetStyle=%[^\n]", instance->state.plasma.widgetStyle.chars);
            sscanf(line, "widgetStyle=\"%[^\"]+", instance->state.plasma.widgetStyle.chars);
        }
        else if(category == PLASMA_CATEGORY_GENERAL)
        {
            sscanf(line, "ColorScheme=%[^\n]", instance->state.plasma.colorScheme.chars);
            sscanf(line, "ColorScheme=\"%[^\"]+", instance->state.plasma.colorScheme.chars);

            sscanf(line, "font=%[^\n]", instance->state.plasma.font.chars);
            sscanf(line, "font=\"%[^\"]+", instance->state.plasma.font.chars);
        }
        else if(category == PLASMA_CATEGORY_ICONS)
        {
            sscanf(line, "Theme=%[^\n]", instance->state.plasma.icons.chars);
            sscanf(line, "Theme=\"%[^\"]+", instance->state.plasma.icons.chars);
        }
    }

    ffStrbufRecalculateLength(&instance->state.plasma.widgetStyle);
    ffStrbufRecalculateLength(&instance->state.plasma.colorScheme);
    ffStrbufRecalculateLength(&instance->state.plasma.icons);
    ffStrbufRecalculateLength(&instance->state.plasma.font);

    if(line != NULL)
        free(line);

    fclose(kdeglobals);
    ffStrbufDestroy(&kdeglobalsFile);

    //In Plasma the default value is never set in the config file, but the whole key-value is discarded.
    ///We must set these values by our self if the file exists (it always does here)
    if(instance->state.plasma.widgetStyle.length == 0)
        ffStrbufAppendS(&instance->state.plasma.widgetStyle, "Breeze");

    if(instance->state.plasma.colorScheme.length == 0)
        ffStrbufAppendS(&instance->state.plasma.colorScheme, "BreezeLight");

    if(instance->state.plasma.icons.length == 0)
        ffStrbufAppendS(&instance->state.plasma.icons, "Breeze");

    if(instance->state.plasma.font.length == 0)
        ffStrbufAppendS(&instance->state.plasma.font, "Noto Sans, 10");

    pthread_mutex_unlock(&mutex);
}
