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

void ffCalculatePlasma(FFinstance* instance, FFstrbuf** themeNamePtr, FFstrbuf** colorNamePtr, FFstrbuf** iconsNamePtr, FFstrbuf** fontNamePtr)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    static FFstrbuf themeName;
    static FFstrbuf colorName;
    static FFstrbuf iconsName;
    static FFstrbuf fontName;
    static bool init = false;

    if(themeNamePtr != NULL)
        *themeNamePtr = &themeName;

    if(colorNamePtr != NULL)
        *colorNamePtr = &colorName;

    if(iconsNamePtr != NULL)
        *iconsNamePtr = &iconsName;

    if(fontNamePtr != NULL)
        *fontNamePtr = &fontName;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return;
    }

    init = true;

    ffStrbufInit(&themeName);
    ffStrbufInit(&colorName);
    ffStrbufInit(&iconsName);
    ffStrbufInit(&fontName);

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
            sscanf(line, "widgetStyle=%[^\n]", themeName.chars);
            sscanf(line, "widgetStyle=\"%[^\"]+", themeName.chars);
        }
        else if(category == PLASMA_CATEGORY_GENERAL)
        {
            sscanf(line, "ColorScheme=%[^\n]", colorName.chars);
            sscanf(line, "ColorScheme=\"%[^\"]+", colorName.chars);

            sscanf(line, "font=%[^\n]", fontName.chars);
            sscanf(line, "font=\"%[^\"]+", fontName.chars);
        }
        else if(category == PLASMA_CATEGORY_ICONS)
        {
            sscanf(line, "Theme=%[^\n]", iconsName.chars);
            sscanf(line, "Theme=\"%[^\"]+", iconsName.chars);
        }
    }

    ffStrbufRecalculateLength(&themeName);
    ffStrbufRecalculateLength(&colorName);
    ffStrbufRecalculateLength(&iconsName);
    ffStrbufRecalculateLength(&fontName);

    if(line != NULL)
        free(line);

    fclose(kdeglobals);
    ffStrbufDestroy(&kdeglobalsFile);

    //In Plasma the default value is never set in the config file, but the whole key-value is discarded.
    ///We must set these values by our self if the file exists (it always does here)
    if(themeName.length == 0)
        ffStrbufAppendS(&themeName, "Breeze");

    if(colorName.length == 0)
        ffStrbufAppendS(&colorName, "BreezeLight");

    if(iconsName.length == 0)
        ffStrbufAppendS(&iconsName, "Breeze");

    if(fontName.length == 0)
        ffStrbufAppendS(&fontName, "Noto Sans, 10");

    pthread_mutex_unlock(&mutex);
}
