#include "fastfetch.h"
#include "util/FFstrbuf.h"
#include <string.h>

#define FF_WMTHEME_MODULE_NAME "WM Theme"
#define FF_WMTHEME_NUM_FORMAT_ARGS 1

static void printWMTheme(FFinstance* instance, const char* theme)
{
    if(instance->config.wmThemeFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey);
        puts(theme);
    }
    else
    {
        ffPrintFormatString(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, NULL, FF_WMTHEME_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, theme}
        });
    }
}

static void printKWin(FFinstance* instance)
{
    char theme[256];
    theme[0] = '\0';
    ffParsePropFileHome(instance, ".config/kwinrc", "theme=%s", theme);

    if(theme[0] == '\0')
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, FF_WMTHEME_NUM_FORMAT_ARGS, "Couldn't find \"theme=\" in \".config/kwinrc\"");
        return;
    }

    printWMTheme(instance, theme);
}

static void printOpenbox(FFinstance* instance)
{
    FFstrbuf absolutePath, theme;
    ffStrbufInitA(&absolutePath, 64);
    ffStrbufAppendS(&absolutePath, instance->state.passwd->pw_dir);
    ffStrbufAppendC(&absolutePath, '/');

    const char* deName = ffGetSessionDesktop();

    if(strcasecmp(deName, (const char*)"LXQt") == 0)
        ffStrbufAppendS(&absolutePath, ".config/openbox/lxqt-rc.xml");
    else if(strcasecmp(deName, (const char*)"LXDE") == 0)
        ffStrbufAppendS(&absolutePath, ".config/openbox/lxde-rc.xml");
    else 
        ffStrbufAppendS(&absolutePath, ".config/openbox/rc.xml");
    
    char* line = NULL;
    size_t len = 0;

    FILE* file = fopen(absolutePath.chars, "r");
    if(file == NULL)
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, FF_WMTHEME_NUM_FORMAT_ARGS, "Couldn't open \"%s\"", absolutePath.chars);
        ffStrbufDestroy(&absolutePath);

        return;
    }
    
    while(getline(&line, &len, file) != -1)
    {
        if(strstr(line, "<theme>") != 0)
            break;
    }
    while(getline(&line, &len, file) != -1)
    {
        if(strstr(line, "<name>") != 0)
        {
            ffStrbufInitA(&theme, 256);
            ffStrbufAppendS(&theme, line);
            ffStrbufRemoveStrings(&theme, 2, "<name>", "</name>");
            ffStrbufTrimRight(&theme, '\n');
            ffStrbufTrim(&theme, ' ');
            break;
        }
        else if(strstr(line, "</theme>") != 0) // sanity check
            break;
    }
    if(line != NULL)
        free(line);

    fclose(file);
        

    if(theme.length == 0)
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, FF_WMTHEME_NUM_FORMAT_ARGS, "Couldn't find theme name in \"%s\"", absolutePath.chars);
    else
        printWMTheme(instance, theme.chars);

    ffStrbufDestroy(&theme);
    ffStrbufDestroy(&absolutePath);
}

void ffPrintWMTheme(FFinstance* instance)
{
    const FFWMResult* result = ffCalculateWM(instance);

    if(result->prettyName.length == 0)
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, FF_WMTHEME_NUM_FORMAT_ARGS, "WM Theme needs sucessfull WM detection");
        return;
    }

    if(ffStrbufIgnCaseCompS(&result->prettyName, "KWin") == 0)
        printKWin(instance);
    else if(ffStrbufIgnCaseCompS(&result->prettyName, "Openbox") == 0)
        printOpenbox(instance);
    else
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, FF_WMTHEME_NUM_FORMAT_ARGS, "Unknown WM: %s", result->prettyName.chars);
}
