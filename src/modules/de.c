#include "fastfetch.h"

#include <string.h>

#define FF_DE_MODULE_NAME "DE"
#define FF_DE_NUM_FORMAT_ARGS 3

static void getKDE(FFstrbuf* name, FFstrbuf* version)
{
    ffStrbufSetS(name, "KDE Plasma");

    char versionBuf[256];
    ffParsePropFile("/usr/share/xsessions/plasma.desktop", "X-KDE-PluginInfo-Version=%[^\n]", versionBuf);

    ffStrbufSetS(version, versionBuf);
}

void ffPrintDesktopEnvironment(FFinstance* instance)
{
    const char* sessionDesktop = ffGetSessionDesktop();

    if(sessionDesktop == NULL)
    {
        ffPrintError(instance, FF_DE_MODULE_NAME, 0, &instance->config.deKey, &instance->config.deFormat, FF_DE_NUM_FORMAT_ARGS, "No relevant XDG_SESSION_* environment variable set");
        return;
    }

    const FFWMResult* wm = ffDetectWM(instance);

    // test if we are running only a WM
    if(
        ffStrbufIgnCaseCompS(&wm->processName, sessionDesktop) == 0 ||
        ffStrbufIgnCaseCompS(&wm->prettyName, sessionDesktop) == 0
    ) return;

    FFstrbuf sessionName;
    ffStrbufInit(&sessionName);

    FFstrbuf sessionVersion;
    ffStrbufInit(&sessionVersion);

    if(strcasecmp(sessionDesktop, "KDE") == 0)
        getKDE(&sessionName, &sessionVersion);

    if(instance->config.deFormat.length == 0)
    {

        ffPrintLogoAndKey(instance, FF_DE_MODULE_NAME, 0, &instance->config.deKey);

        if(sessionName.length > 0)
            ffStrbufWriteTo(&sessionName, stdout);
        else
            fputs(sessionDesktop, stdout);

        if(sessionVersion.length > 0)
        {
            putchar(' ');
            ffStrbufWriteTo(&sessionVersion, stdout);
        }

        putchar('\n');
    }
    else
    {
        ffPrintFormatString(instance, FF_DE_MODULE_NAME, 0, &instance->config.deKey, &instance->config.deFormat, NULL, FF_DE_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, sessionDesktop},
            {FF_FORMAT_ARG_TYPE_STRBUF, &sessionName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &sessionVersion},
        });
    }

    ffStrbufDestroy(&sessionName);
    ffStrbufDestroy(&sessionVersion);
}
