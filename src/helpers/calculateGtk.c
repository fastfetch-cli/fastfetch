#include "fastfetch.h"

#include "dconf/client/dconf-client.h"
#include "dlfcn.h"

static inline bool allPropertiesSet(FFstrbuf* themeNamePtr, FFstrbuf* iconsNamePtr, FFstrbuf* fontNamePtr)
{
    return
        themeNamePtr->length > 0 &&
        iconsNamePtr->length > 0 &&
        fontNamePtr->length > 0;
}

static void parseGTKDConfSettings(FFinstance* instance, FFstrbuf* themeNamePtr, FFstrbuf* iconsNamePtr, FFstrbuf* fontNamePtr)
{
    static const gchar* themeName;
    static const gchar* iconsName;
    static const gchar* fontName;

    static bool init = false;
    if(init)
    {
        ffStrbufAppendS(themeNamePtr, themeName);
        ffStrbufAppendS(iconsNamePtr, iconsName);
        ffStrbufAppendS(fontNamePtr, fontName);
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

    ffStrbufAppendS(themeNamePtr, themeName);
    ffStrbufAppendS(iconsNamePtr, iconsName);
    ffStrbufAppendS(fontNamePtr, fontName);
}

static void parseGTKConfigFile(FFstrbuf* fileName, FFstrbuf* themeNamePtr, FFstrbuf* iconsNamePtr, FFstrbuf* fontNamePtr)
{
    FILE* file = fopen(fileName->chars, "r");
    if(file == NULL)
        return;

    char* line = NULL;
    size_t len = 0;

    while(getline(&line, &len, file) != -1)
    {
        if(themeNamePtr->length == 0)
        {
            sscanf(line, "gtk-theme-name=%[^\n]", themeNamePtr->chars);
            sscanf(line, "gtk-theme-name=\"%[^\"]+", themeNamePtr->chars);
        }

        if(iconsNamePtr->length == 0)
        {
            sscanf(line, "gtk-icon-theme-name=%[^\n]", iconsNamePtr->chars);
            sscanf(line, "gtk-icon-theme-name=\"%[^\"]+", iconsNamePtr->chars);
        }

        if(fontNamePtr->length == 0)
        {
            sscanf(line, "gtk-font-name=%[^\n]", fontNamePtr->chars);
            sscanf(line, "gtk-font-name=\"%[^\"]+", fontNamePtr->chars);
        }
    }

    if(line != NULL)
        free(line);

    fclose(file);

    ffStrbufRecalculateLength(themeNamePtr);
    ffStrbufRecalculateLength(iconsNamePtr);
    ffStrbufRecalculateLength(fontNamePtr);
}

static void calculateGTKFromConfigDir(const char* configDir, const char* version, FFstrbuf* themeNamePtr, FFstrbuf* iconsNamePtr, FFstrbuf* fontNamePtr)
{
    // <configdir>/gtk-<version>.0/settings.ini
    FFstrbuf file1;
    ffStrbufInitA(&file1, 64);
    ffStrbufAppendS(&file1, configDir);
    ffStrbufAppendS(&file1, "gtk-");
    ffStrbufAppendS(&file1, version);
    ffStrbufAppendS(&file1, ".0/settings.ini");
    parseGTKConfigFile(&file1, themeNamePtr, iconsNamePtr, fontNamePtr);
    ffStrbufDestroy(&file1);
    if(allPropertiesSet(themeNamePtr, iconsNamePtr, fontNamePtr))
        return;

    // <configdir>/gtk-<version>.0/gtkrc
    FFstrbuf file2;
    ffStrbufInitA(&file2, 64);
    ffStrbufAppendS(&file2, configDir);
    ffStrbufAppendS(&file2, "gtk-");
    ffStrbufAppendS(&file2, version);
    ffStrbufAppendS(&file2, ".0/gtkrc");
    parseGTKConfigFile(&file2, themeNamePtr, iconsNamePtr, fontNamePtr);
    ffStrbufDestroy(&file2);
    if(allPropertiesSet(themeNamePtr, iconsNamePtr, fontNamePtr))
        return;

    // <configdir>/gtkrc-<version>.0
    FFstrbuf file3;
    ffStrbufInitA(&file3, 64);
    ffStrbufAppendS(&file3, configDir);
    ffStrbufAppendS(&file3, "gtkrc-");
    ffStrbufAppendS(&file3, version);
    ffStrbufAppendS(&file3, ".0");
    parseGTKConfigFile(&file3, themeNamePtr, iconsNamePtr, fontNamePtr);
    ffStrbufDestroy(&file3);
    if(allPropertiesSet(themeNamePtr, iconsNamePtr, fontNamePtr))
        return;

    // <configdir>/.gtkrc-<version>.0
    FFstrbuf file4;
    ffStrbufInitA(&file4, 64);
    ffStrbufAppendS(&file4, configDir);
    ffStrbufAppendS(&file4, ".gtkrc-");
    ffStrbufAppendS(&file4, version);
    ffStrbufAppendS(&file4, ".0");
    parseGTKConfigFile(&file4, themeNamePtr, iconsNamePtr, fontNamePtr);
    ffStrbufDestroy(&file4);
}

static void calculateGTK(FFinstance* instance, const char* version, FFstrbuf* themeNamePtr, FFstrbuf* iconsNamePtr, FFstrbuf* fontNamePtr)
{
    static FFstrbuf configDir;
    static bool configDirInit = false;

    if(!configDirInit)
    {
        ffStrbufInitA(&configDir, 64);
        ffStrbufAppendS(&configDir, instance->state.passwd->pw_dir);
        ffStrbufAppendS(&configDir, "/.config/");
        configDirInit = true;
    }

    static FFstrbuf xdgConfigDir;
    static bool xdgConfigDirInit = false;

    if(!xdgConfigDirInit)
    {
        ffStrbufInitA(&xdgConfigDir, 64);
        ffStrbufAppendS(&xdgConfigDir, getenv("XDG_CONFIG_HOME"));
        if(xdgConfigDir.length > 0 && xdgConfigDir.chars[xdgConfigDir.length - 1] != '/')
            ffStrbufAppendC(&xdgConfigDir, '/');
        xdgConfigDirInit = true;
    }

    static bool xdgIsDifferent;
    static bool xdgIsDifferentInit = false;

    if(!xdgIsDifferentInit)
    {
        xdgIsDifferent = ffStrbufComp(&configDir, &xdgConfigDir) != 0;
        xdgConfigDirInit = true;
    }

    if(xdgIsDifferent)
    {
        calculateGTKFromConfigDir(xdgConfigDir.chars, version, themeNamePtr, iconsNamePtr, fontNamePtr);
        if(allPropertiesSet(themeNamePtr, iconsNamePtr, fontNamePtr))
            return;
    }

    calculateGTKFromConfigDir(configDir.chars, version, themeNamePtr, iconsNamePtr, fontNamePtr);
    if(allPropertiesSet(themeNamePtr, iconsNamePtr, fontNamePtr))
            return;

    // <homedir>/gtkrc-<version>.0
    FFstrbuf homeConfig1;
    ffStrbufInitA(&homeConfig1, 64);
    ffStrbufAppendS(&homeConfig1, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&homeConfig1, "/gtkrc-");
    ffStrbufAppendS(&homeConfig1, version);
    ffStrbufAppendS(&homeConfig1, ".0");
    parseGTKConfigFile(&homeConfig1, themeNamePtr, iconsNamePtr, fontNamePtr);
    ffStrbufDestroy(&homeConfig1);
    if(allPropertiesSet(themeNamePtr, iconsNamePtr, fontNamePtr))
            return;

    // <homedir>/.gtkrc-<version>.0
    FFstrbuf homeConfig2;
    ffStrbufInitA(&homeConfig2, 64);
    ffStrbufAppendS(&homeConfig2, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&homeConfig2, "/.gtkrc-");
    ffStrbufAppendS(&homeConfig2, version);
    ffStrbufAppendS(&homeConfig2, ".0");
    parseGTKConfigFile(&homeConfig2, themeNamePtr, iconsNamePtr, fontNamePtr);
    ffStrbufDestroy(&homeConfig2);
    if(allPropertiesSet(themeNamePtr, iconsNamePtr, fontNamePtr))
            return;

    parseGTKDConfSettings(instance, themeNamePtr, iconsNamePtr, fontNamePtr);
    if(allPropertiesSet(themeNamePtr, iconsNamePtr, fontNamePtr))
        return;

    calculateGTKFromConfigDir("/etc/", version, themeNamePtr, iconsNamePtr, fontNamePtr);
}

#define FF_CALCULATE_GTK_IMPL(version) \
    static FFstrbuf themeName; \
    static FFstrbuf iconsName; \
    static FFstrbuf fontName; \
    if(themeNamePtr != NULL) *themeNamePtr = &themeName; \
    if(iconsNamePtr != NULL) *iconsNamePtr = &iconsName; \
    if(fontNamePtr != NULL) *fontNamePtr = &fontName; \
    static bool init = false; \
    if(init) return; \
    init = true; \
    ffStrbufInit(&themeName); \
    ffStrbufInit(&iconsName); \
    ffStrbufInit(&fontName); \
    calculateGTK(instance, #version, &themeName, &iconsName, &fontName);

void ffCalculateGTK2(FFinstance* instance, FFstrbuf** themeNamePtr, FFstrbuf** iconsNamePtr, FFstrbuf** fontNamePtr)
{
    FF_CALCULATE_GTK_IMPL(2)
}

void ffCalculateGTK3(FFinstance* instance, FFstrbuf** themeNamePtr, FFstrbuf** iconsNamePtr, FFstrbuf** fontNamePtr)
{
    FF_CALCULATE_GTK_IMPL(3)
}

void ffCalculateGTK4(FFinstance* instance, FFstrbuf** themeNamePtr, FFstrbuf** iconsNamePtr, FFstrbuf** fontNamePtr)
{
    FF_CALCULATE_GTK_IMPL(4)
}

#undef FF_CALCULATE_GTK_IMPL
