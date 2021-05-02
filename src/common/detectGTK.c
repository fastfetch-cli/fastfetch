#include "fastfetch.h"

#include <dlfcn.h>
#include <pthread.h>

static inline bool allPropertiesSet(FFGTKResult* result)
{
    return
        result->theme.length > 0 &&
        result->icons.length > 0 &&
        result->font.length > 0;
}

static inline void initresult(FFGTKResult* result)
{
    ffStrbufInit(&result->theme);
    ffStrbufInit(&result->icons);
    ffStrbufInit(&result->font);
}

static inline void applyGTKDConfSettings(FFGTKResult* result, const char* themeName, const char* iconsName, const char* fontName)
{
    if(result->theme.length == 0)
        ffStrbufAppendS(&result->theme, themeName);

    if(result->icons.length == 0)
        ffStrbufAppendS(&result->icons, iconsName);

    if(result->font.length == 0)
        ffStrbufAppendS(&result->font, fontName);
}

static void parseGTKDConfSettings(FFinstance* instance, FFGTKResult* result)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    static const char* themeName;
    static const char* iconsName;
    static const char* fontName;

    static bool init = false;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        applyGTKDConfSettings(result, themeName, iconsName, fontName);
        return;
    }

    init = true;

    themeName = ffGSettingsGetValue(instance, "org.gnome.desktop.interface", "gtk-theme");
    iconsName = ffGSettingsGetValue(instance, "org.gnome.desktop.interface", "icon-theme");
    fontName = ffGSettingsGetValue(instance, "org.gnome.desktop.interface", "font-name");

    pthread_mutex_unlock(&mutex);
    applyGTKDConfSettings(result, themeName, iconsName, fontName);
}

static void parseGTKConfigFile(FFstrbuf* fileName, FFGTKResult* result)
{
    FILE* file = fopen(fileName->chars, "r");
    if(file == NULL)
        return;

    char* line = NULL;
    size_t len = 0;

    while(getline(&line, &len, file) != -1)
    {
        if(result->theme.length == 0)
        {
            sscanf(line, "gtk-theme-name=%[^\n]", result->theme.chars);
            sscanf(line, "gtk-theme-name=\"%[^\"]+", result->theme.chars);
        }

        if(result->icons.length == 0)
        {
            sscanf(line, "gtk-icon-theme-name=%[^\n]", result->icons.chars);
            sscanf(line, "gtk-icon-theme-name=\"%[^\"]+", result->icons.chars);
        }

        if(result->font.length == 0)
        {
            sscanf(line, "gtk-font-name=%[^\n]", result->font.chars);
            sscanf(line, "gtk-font-name=\"%[^\"]+", result->font.chars);
        }
    }

    if(line != NULL)
        free(line);

    fclose(file);

    ffStrbufRecalculateLength(&result->theme);
    ffStrbufRecalculateLength(&result->icons);
    ffStrbufRecalculateLength(&result->font);
}

static void detectGTKFromConfigDir(const char* configDir, const char* version, FFGTKResult* result)
{
    // <configdir>/gtk-<version>.0/settings.ini
    FFstrbuf file1;
    ffStrbufInitA(&file1, 64);
    ffStrbufAppendS(&file1, configDir);
    ffStrbufAppendS(&file1, "/gtk-");
    ffStrbufAppendS(&file1, version);
    ffStrbufAppendS(&file1, ".0/settings.ini");
    parseGTKConfigFile(&file1, result);
    ffStrbufDestroy(&file1);
    if(allPropertiesSet(result))
        return;

    // <configdir>/gtk-<version>.0/gtkrc
    FFstrbuf file2;
    ffStrbufInitA(&file2, 64);
    ffStrbufAppendS(&file2, configDir);
    ffStrbufAppendS(&file2, "/gtk-");
    ffStrbufAppendS(&file2, version);
    ffStrbufAppendS(&file2, ".0/gtkrc");
    parseGTKConfigFile(&file2, result);
    ffStrbufDestroy(&file2);
    if(allPropertiesSet(result))
        return;

    // <configdir>/gtkrc-<version>.0
    FFstrbuf file3;
    ffStrbufInitA(&file3, 64);
    ffStrbufAppendS(&file3, configDir);
    ffStrbufAppendS(&file3, "/gtkrc-");
    ffStrbufAppendS(&file3, version);
    ffStrbufAppendS(&file3, ".0");
    parseGTKConfigFile(&file3, result);
    ffStrbufDestroy(&file3);
    if(allPropertiesSet(result))
        return;

    // <configdir>/.gtkrc-<version>.0
    FFstrbuf file4;
    ffStrbufInitA(&file4, 64);
    ffStrbufAppendS(&file4, configDir);
    ffStrbufAppendS(&file4, "/.gtkrc-");
    ffStrbufAppendS(&file4, version);
    ffStrbufAppendS(&file4, ".0");
    parseGTKConfigFile(&file4, result);
    ffStrbufDestroy(&file4);
}

static void detectGTK(FFinstance* instance, const char* version, FFGTKResult* result)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    static FFstrbuf configDir;
    static FFstrbuf xdgConfigDir;
    static bool xdgIsDifferent;
    static bool init = false;

    initresult(result);

    pthread_mutex_lock(&mutex);

    if(!init)
    {
        ffStrbufInitA(&configDir, 64);
        ffStrbufAppendS(&configDir, instance->state.passwd->pw_dir);
        ffStrbufAppendS(&configDir, "/.config");

        ffStrbufInitA(&xdgConfigDir, 64);
        ffStrbufAppendS(&xdgConfigDir, getenv("XDG_CONFIG_HOME"));
        ffStrbufTrimRight(&xdgConfigDir, '/');

        xdgIsDifferent = ffStrbufComp(&configDir, &xdgConfigDir) != 0;

        init = true;
    }

    pthread_mutex_unlock(&mutex);

    if(xdgIsDifferent)
    {
        // $XDG_CONFIG_HOME
        detectGTKFromConfigDir(xdgConfigDir.chars, version, result);
        if(allPropertiesSet(result))
            return;
    }

    // /home/<user>/.config
    detectGTKFromConfigDir(configDir.chars, version, result);
    if(allPropertiesSet(result))
            return;

    // /home/<user>
    detectGTKFromConfigDir(instance->state.passwd->pw_dir, version, result);
    if(allPropertiesSet(result))
            return;

    parseGTKDConfSettings(instance, result);
    if(allPropertiesSet(result))
        return;

    // /etc
    detectGTKFromConfigDir("/etc/", version, result);
}

#define FF_CALCULATE_GTK_IMPL(version) \
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; \
    static FFGTKResult result; \
    static bool init = false; \
    pthread_mutex_lock(&mutex); \
    if(init){ \
        pthread_mutex_unlock(&mutex);\
        return &result; \
    } \
    init = true; \
    ffStrbufInit(&result.theme); \
    ffStrbufInit(&result.icons); \
    ffStrbufInit(&result.font); \
    detectGTK(instance, #version, &result); \
    pthread_mutex_unlock(&mutex); \
    return &result;

const FFGTKResult* ffDetectGTK2(FFinstance* instance)
{
    FF_CALCULATE_GTK_IMPL(2)
}

const FFGTKResult* ffDetectGTK3(FFinstance* instance)
{
    FF_CALCULATE_GTK_IMPL(3)
}

const FFGTKResult* ffDetectGTK4(FFinstance* instance)
{
    FF_CALCULATE_GTK_IMPL(4)
}

#undef FF_CALCULATE_GTK_IMPL
