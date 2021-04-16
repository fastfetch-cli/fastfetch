#include "fastfetch.h"

#include <dlfcn.h>
#include <pthread.h>
#include <dconf/client/dconf-client.h>

static inline bool allPropertiesSet(FFgtkval* gtkval)
{
    return
        gtkval->theme.length > 0 &&
        gtkval->icons.length > 0 &&
        gtkval->font.length > 0;
}

static inline void initGtkval(FFgtkval* gtkval)
{
    ffStrbufInit(&gtkval->theme);
    ffStrbufInit(&gtkval->icons);
    ffStrbufInit(&gtkval->font);
}

static inline void applyGTKDConfSettings(FFgtkval* gtkval, const gchar* themeName, const gchar* iconsName, const gchar* fontName)
{
    if(gtkval->theme.length == 0)
        ffStrbufAppendS(&gtkval->theme, themeName);

    if(gtkval->icons.length == 0)
        ffStrbufAppendS(&gtkval->icons, iconsName);

    if(gtkval->font.length == 0)
        ffStrbufAppendS(&gtkval->font, fontName);
}

static void parseGTKDConfSettings(FFinstance* instance, FFgtkval* gtkval)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    static const gchar* themeName;
    static const gchar* iconsName;
    static const gchar* fontName;

    static bool init = false;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        applyGTKDConfSettings(gtkval, themeName, iconsName, fontName);
        pthread_mutex_unlock(&mutex);
        return;
    }
    init = true;

    void* dconf;
    if(instance->config.libDConf.length == 0)
        dconf = dlopen("libdconf.so", RTLD_LAZY);
    else
        dconf = dlopen(instance->config.libDConf.chars, RTLD_LAZY);

    if(dconf == NULL)
        return;

    DConfClient*(*ffdconf_client_new)(void) = dlsym(dconf, "dconf_client_new");
    if(ffdconf_client_new == NULL)
    {
        dlclose(dconf);
        return;
    }

    const gchar*(*ffg_variant_get_string)(GVariant*, gsize*) = dlsym(dconf, "g_variant_get_string");
    if(ffg_variant_get_string == NULL)
    {
        dlclose(dconf);
        return;
    }

    GVariant*(*ffdconf_client_read)(DConfClient*, const gchar*) = dlsym(dconf, "dconf_client_read");
    if(ffdconf_client_read == NULL)
    {
        dlclose(dconf);
        return;
    }

    DConfClient* dconfClient = ffdconf_client_new();
    if(dconfClient == NULL)
    {
        dlclose(dconf);
        return;
    }

    GVariant* themeNameVariant = ffdconf_client_read(dconfClient, "/org/gnome/desktop/interface/gtk-theme");
    if(themeNameVariant != NULL)
        themeName = ffg_variant_get_string(themeNameVariant, NULL);

    GVariant* fontNameVariant = ffdconf_client_read(dconfClient, "/org/gnome/desktop/interface/font-name");
    if(fontNameVariant != NULL)
        fontName = ffg_variant_get_string(fontNameVariant, NULL);

    GVariant* iconsNameVariant = ffdconf_client_read(dconfClient, "/org/gnome/desktop/interface/icon-theme");
    if(iconsNameVariant != NULL)
        iconsName = ffg_variant_get_string(iconsNameVariant, NULL);

    dlclose(dconf);

    applyGTKDConfSettings(gtkval, themeName, iconsName, fontName);
    pthread_mutex_unlock(&mutex);
}

static void parseGTKConfigFile(FFstrbuf* fileName, FFgtkval* gtkval)
{
    FILE* file = fopen(fileName->chars, "r");
    if(file == NULL)
        return;

    char* line = NULL;
    size_t len = 0;

    while(getline(&line, &len, file) != -1)
    {
        if(gtkval->theme.length == 0)
        {
            sscanf(line, "gtk-theme-name=%[^\n]", gtkval->theme.chars);
            sscanf(line, "gtk-theme-name=\"%[^\"]+", gtkval->theme.chars);
        }

        if(gtkval->icons.length == 0)
        {
            sscanf(line, "gtk-icon-theme-name=%[^\n]", gtkval->icons.chars);
            sscanf(line, "gtk-icon-theme-name=\"%[^\"]+", gtkval->icons.chars);
        }

        if(gtkval->font.length == 0)
        {
            sscanf(line, "gtk-font-name=%[^\n]", gtkval->font.chars);
            sscanf(line, "gtk-font-name=\"%[^\"]+", gtkval->font.chars);
        }
    }

    if(line != NULL)
        free(line);

    fclose(file);

    ffStrbufRecalculateLength(&gtkval->theme);
    ffStrbufRecalculateLength(&gtkval->icons);
    ffStrbufRecalculateLength(&gtkval->font);
}

static void calculateGTKFromConfigDir(const char* configDir, const char* version, FFgtkval* gtkval)
{
    // <configdir>/gtk-<version>.0/settings.ini
    FFstrbuf file1;
    ffStrbufInitA(&file1, 64);
    ffStrbufAppendS(&file1, configDir);
    ffStrbufAppendS(&file1, "/gtk-");
    ffStrbufAppendS(&file1, version);
    ffStrbufAppendS(&file1, ".0/settings.ini");
    parseGTKConfigFile(&file1, gtkval);
    ffStrbufDestroy(&file1);
    if(allPropertiesSet(gtkval))
        return;

    // <configdir>/gtk-<version>.0/gtkrc
    FFstrbuf file2;
    ffStrbufInitA(&file2, 64);
    ffStrbufAppendS(&file2, configDir);
    ffStrbufAppendS(&file2, "/gtk-");
    ffStrbufAppendS(&file2, version);
    ffStrbufAppendS(&file2, ".0/gtkrc");
    parseGTKConfigFile(&file2, gtkval);
    ffStrbufDestroy(&file2);
    if(allPropertiesSet(gtkval))
        return;

    // <configdir>/gtkrc-<version>.0
    FFstrbuf file3;
    ffStrbufInitA(&file3, 64);
    ffStrbufAppendS(&file3, configDir);
    ffStrbufAppendS(&file3, "/gtkrc-");
    ffStrbufAppendS(&file3, version);
    ffStrbufAppendS(&file3, ".0");
    parseGTKConfigFile(&file3, gtkval);
    ffStrbufDestroy(&file3);
    if(allPropertiesSet(gtkval))
        return;

    // <configdir>/.gtkrc-<version>.0
    FFstrbuf file4;
    ffStrbufInitA(&file4, 64);
    ffStrbufAppendS(&file4, configDir);
    ffStrbufAppendS(&file4, "/.gtkrc-");
    ffStrbufAppendS(&file4, version);
    ffStrbufAppendS(&file4, ".0");
    parseGTKConfigFile(&file4, gtkval);
    ffStrbufDestroy(&file4);
}

static void calculateGTK(FFinstance* instance, const char* version, FFgtkval* gtkval)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    static FFstrbuf configDir;
    static FFstrbuf xdgConfigDir;
    static bool xdgIsDifferent;
    static bool init = false;

    initGtkval(gtkval);

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
        calculateGTKFromConfigDir(xdgConfigDir.chars, version, gtkval);
        if(allPropertiesSet(gtkval))
            return;
    }

    // /home/<user>/.config
    calculateGTKFromConfigDir(configDir.chars, version, gtkval);
    if(allPropertiesSet(gtkval))
            return;

    // /home/<user>
    calculateGTKFromConfigDir(instance->state.passwd->pw_dir, version, gtkval);
    if(allPropertiesSet(gtkval))
            return;

    parseGTKDConfSettings(instance, gtkval);
    if(allPropertiesSet(gtkval))
        return;

    // /etc
    calculateGTKFromConfigDir("/etc/", version, gtkval);
}

#define FF_CALCULATE_GTK_IMPL(version) \
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; \
    static bool init = false; \
    pthread_mutex_lock(&mutex); \
    if(init){ \
        pthread_mutex_unlock(&mutex);\
        return; \
    } \
    init = true; \
    calculateGTK(instance, #version, &instance->state.gtk##version); \
    pthread_mutex_unlock(&mutex);

void ffCalculateGTK2(FFinstance* instance)
{
    FF_CALCULATE_GTK_IMPL(2)
}

void ffCalculateGTK3(FFinstance* instance)
{
    FF_CALCULATE_GTK_IMPL(3)
}

void ffCalculateGTK4(FFinstance* instance)
{
    FF_CALCULATE_GTK_IMPL(4)
}

#undef FF_CALCULATE_GTK_IMPL
