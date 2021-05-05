#include "fastfetch.h"
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

static void printMutter(FFinstance* instance)
{
    const char* theme = ffSettingsGet(instance, "/org/gnome/shell/extensions/user-theme/name", "org.gnome.shell.extensions.user-theme", NULL, "name");

    if(theme == NULL)
        theme = ffSettingsGet(instance, "/org/gnome/desktop/wm/preferences/theme", "org.gnome.desktop.wm.preferences", NULL, "theme");

    if(theme == NULL)
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, FF_WMTHEME_NUM_FORMAT_ARGS, "Couldn't found mutter theme in GSettings / DConf");
        return;
    }

    printWMTheme(instance, theme);
}

static void printOpenbox(FFinstance* instance, const FFstrbuf* dePrettyName)
{
    FFstrbuf absolutePath;
    ffStrbufInitA(&absolutePath, 64);
    ffStrbufAppendS(&absolutePath, instance->state.passwd->pw_dir);
    ffStrbufAppendC(&absolutePath, '/');

    if(ffStrbufIgnCaseCompS(dePrettyName, "LXQT") == 0)
        ffStrbufAppendS(&absolutePath, ".config/openbox/lxqt-rc.xml");
    else if(ffStrbufIgnCaseCompS(dePrettyName, "LXDE") == 0)
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

    FFstrbuf theme;
    ffStrbufInitA(&theme, 256);

    while(getline(&line, &len, file) != -1)
    {
        if(strstr(line, "<theme>") != 0)
            break;
    }

    while(getline(&line, &len, file) != -1)
    {
        if(strstr(line, "<name>") != 0)
        {
            ffStrbufAppendS(&theme, line);
            ffStrbufRemoveStrings(&theme, 2, "<name>", "</name>");
            ffStrbufTrimRight(&theme, '\n');
            ffStrbufTrim(&theme, ' ');
            break;
        }

        if(strstr(line, "</theme>") != 0) // sanity check
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
    const FFWMDEResult* result = ffDetectWMDE(instance);

    if(result->wmPrettyName.length == 0)
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, FF_WMTHEME_NUM_FORMAT_ARGS, "WM Theme needs sucessfull WM detection");
        return;
    }

    if(ffStrbufIgnCaseCompS(&result->wmPrettyName, "KWin") == 0 || ffStrbufIgnCaseCompS(&result->wmPrettyName, "KDE") == 0 || ffStrbufIgnCaseCompS(&result->wmPrettyName, "Plasma") == 0)
        printKWin(instance);
    else if(ffStrbufIgnCaseCompS(&result->wmPrettyName, "Mutter") == 0 || ffStrbufIgnCaseCompS(&result->wmPrettyName, "Gnome") == 0 || ffStrbufIgnCaseCompS(&result->wmPrettyName, "Ubuntu") == 0)
        printMutter(instance);
    else if(ffStrbufIgnCaseCompS(&result->wmPrettyName, "Openbox") == 0)
        printOpenbox(instance, &result->dePrettyName);
    else
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, FF_WMTHEME_NUM_FORMAT_ARGS, "Unknown WM: %s", result->dePrettyName.chars);
}
