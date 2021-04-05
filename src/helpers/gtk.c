#include "fastfetch.h"

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

    // <configdir>/gtk-<version>.0/gtkrc
    FFstrbuf file2;
    ffStrbufInitA(&file2, 64);
    ffStrbufAppendS(&file2, configDir);
    ffStrbufAppendS(&file2, "gtk-");
    ffStrbufAppendS(&file2, version);
    ffStrbufAppendS(&file2, ".0/gtkrc");
    parseGTKConfigFile(&file2, themeNamePtr, iconsNamePtr, fontNamePtr);
    ffStrbufDestroy(&file2);

    // <configdir>/gtkrc-<version>.0
    FFstrbuf file3;
    ffStrbufInitA(&file3, 64);
    ffStrbufAppendS(&file3, configDir);
    ffStrbufAppendS(&file3, "gtkrc-");
    ffStrbufAppendS(&file3, version);
    ffStrbufAppendS(&file3, ".0");
    parseGTKConfigFile(&file3, themeNamePtr, iconsNamePtr, fontNamePtr);
    ffStrbufDestroy(&file3);

    // <configdir>/.gtkrc-<version>.0
    FFstrbuf file4;
    ffStrbufInitA(&file4, 64);
    ffStrbufAppendS(&file4, configDir);
    ffStrbufAppendS(&file4, "gtkrc-");
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
        calculateGTKFromConfigDir(xdgConfigDir.chars, version, themeNamePtr, iconsNamePtr, fontNamePtr);

    calculateGTKFromConfigDir(configDir.chars, version, themeNamePtr, iconsNamePtr, fontNamePtr);

    // <homedir>/gtkrc-<version>.0
    FFstrbuf homeConfig1;
    ffStrbufInitA(&homeConfig1, 64);
    ffStrbufAppendS(&homeConfig1, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&homeConfig1, "/gtkrc-");
    ffStrbufAppendS(&homeConfig1, version);
    ffStrbufAppendS(&homeConfig1, ".0");
    parseGTKConfigFile(&homeConfig1, themeNamePtr, iconsNamePtr, fontNamePtr);
    ffStrbufDestroy(&homeConfig1);

    // <homedir>/.gtkrc-<version>.0
    FFstrbuf homeConfig2;
    ffStrbufInitA(&homeConfig2, 64);
    ffStrbufAppendS(&homeConfig2, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&homeConfig2, "/.gtkrc-");
    ffStrbufAppendS(&homeConfig2, version);
    ffStrbufAppendS(&homeConfig2, ".0");
    parseGTKConfigFile(&homeConfig2, themeNamePtr, iconsNamePtr, fontNamePtr);
    ffStrbufDestroy(&homeConfig2);

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

void ffFormatGtkPretty(FFstrbuf* buffer, FFstrbuf* gtk2, FFstrbuf* gtk3, FFstrbuf* gtk4)
{
    if(gtk2->length > 0 && gtk3->length > 0 && gtk4->length > 0)
    {
        if((ffStrbufIgnCaseComp(gtk2, gtk3) == 0) && (ffStrbufIgnCaseComp(gtk2, gtk4) == 0))
        {
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK2/3/4]");
        }
        else if(ffStrbufIgnCaseComp(gtk2, gtk3) == 0)
        {
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK2/3], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK4]");
        }
        else if(ffStrbufIgnCaseComp(gtk3, gtk4) == 0)
        {
            ffStrbufAppend(buffer, gtk2);
            ffStrbufAppendS(buffer, " [GTK2], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK3/4]");
        }
        else
        {
            ffStrbufAppend(buffer, gtk2);
            ffStrbufAppendS(buffer, " [GTK2], ");
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK3], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK4]");
        }
    }
    else if(gtk2->length > 0 && gtk3->length > 0)
    {
        if(ffStrbufIgnCaseComp(gtk2, gtk3) == 0)
        {
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK2/3]");
        }
        else
        {
            ffStrbufAppend(buffer, gtk2);
            ffStrbufAppendS(buffer, " [GTK2], ");
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK3]");
        }
    }
    else if(gtk3->length > 0 && gtk4->length > 0)
    {
        if(ffStrbufIgnCaseComp(gtk3, gtk4) == 0)
        {
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK3/4]");
        }
        else
        {
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK3], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK4]");
        }
    }
    else if(gtk2->length > 0)
    {
        ffStrbufAppend(buffer, gtk2);
        ffStrbufAppendS(buffer, " [GTK2]");
    }
    else if(gtk3->length > 0)
    {
        ffStrbufAppend(buffer, gtk3);
        ffStrbufAppendS(buffer, " [GTK3]");
    }
    else if(gtk4->length > 0)
    {
        ffStrbufAppend(buffer, gtk4);
        ffStrbufAppendS(buffer, " [GTK4]");
    }
}
