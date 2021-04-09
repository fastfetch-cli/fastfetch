#include "fastfetch.h"

#include <pthread.h>

void ffCalculatePlasma(FFinstance* instance, FFstrbuf** themeNamePtr, FFstrbuf** iconsNamePtr, FFstrbuf** fontNamePtr)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    static FFstrbuf themeName;
    static FFstrbuf iconsName;
    static FFstrbuf fontName;
    static bool init = false;

    if(themeNamePtr != NULL)
        *themeNamePtr = &themeName;

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

    while(getline(&line, &len, kdeglobals) != -1)
    {
        sscanf(line, "widgetStyle=%[^\n]", themeName.chars);
        sscanf(line, "widgetStyle=\"%[^\"]+", themeName.chars);

        sscanf(line, "Theme=%[^\n]", iconsName.chars);
        sscanf(line, "Theme=\"%[^\"]+", iconsName.chars);

        sscanf(line, "font=%[^\n]", fontName.chars);
        sscanf(line, "font=\"%[^\"]+", fontName.chars);
    }

    ffStrbufRecalculateLength(&themeName);
    ffStrbufRecalculateLength(&iconsName);
    ffStrbufRecalculateLength(&fontName);

    if(line != NULL)
        free(line);

    fclose(kdeglobals);
    ffStrbufDestroy(&kdeglobalsFile);

    //When using Noto Sans 10, the default font in KDE Plasma, the font entry is deleted from the config file instead of set.
    //So the pure existence of the file sets this font value if not set other in the file itself.
    if(fontName.length == 0)
        ffStrbufAppendS(&fontName, "Noto Sans, 10");

    pthread_mutex_unlock(&mutex);
}
