#include "fastfetch.h"
#include "string.h"
#include "util/FFstrbuf.h"

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

void ffGetOBThemeName(FFinstance* instance, const char* fName, char* buffer)
{
    FFstrbuf absolutePath, themeStrbuf;
    ffStrbufInitA(&absolutePath, 64);
    ffStrbufAppendS(&absolutePath, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&absolutePath, fName);
    ffStrbufInitA(&themeStrbuf, 256);
    
    char* line = NULL;
    size_t len = 0;

    FILE* file = fopen(absolutePath.chars, "r");
    if(file == NULL)
        return; // handle errors in higher functions
        
    while (getline(&line, &len, file) != -1)
    {
        if (strstr(line, "<theme>") != 0)
            break;
    }
    while (getline(&line, &len, file) != -1)
    {
        if (strstr(line, "<name>") != 0)
        {
            const char* delStrs[] = {"<name>", "</name>"};
            
            ffStrbufAppendS(&themeStrbuf, line);
            ffStrbufRemoveStringsA(&themeStrbuf, 2, delStrs);
            ffStrbufTrimRight(&themeStrbuf, '\n');
            ffStrbufTrim(&themeStrbuf, ' ');

            strcpy(buffer, themeStrbuf.chars);
        }
        else if (strstr(line, "</theme>") != 0)
            break;
        break;
    }
        
    fclose(file);
    if(line != NULL)
        free(line);

    ffStrbufDestroy(&absolutePath);
    ffStrbufDestroy(&themeStrbuf);
}

static void printOpenbox(FFinstance* instance)
{
	char relPath[64];
	char theme[256];
	theme[0] = '\0';

	const char* deName = getenv("XDG_SESSION_DESKTOP");

	if (strcmp(deName, (const char*)"LXQt Desktop") == 0)
		strcpy(relPath, "/.config/openbox/lxqt-rc.xml");
	else if(strcmp(deName, (const char*)"LXDE") == 0)
		strcpy(relPath, "/.config/openbox/lxde-rc.xml");
	else 
		strcpy(relPath, "/.config/openbox/rc.xml");

	ffGetOBThemeName(instance, relPath, theme);

	if(theme[0] == '\0')
	{
		ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, FF_WMTHEME_NUM_FORMAT_ARGS, "Couldn't find theme name in \"%s\"", relPath);
		return;
	}

	printWMTheme(instance, theme);
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
