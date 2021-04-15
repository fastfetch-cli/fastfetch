#include "fastfetch.h"

#include <string.h>

#define FF_DE_MODULE_NAME "DE"
#define FF_DE_NUM_FORMAT_ARGS 3

static void getKDE(FFstrbuf* name, FFstrbuf* version, FFstrbuf* type)
{
    UNUSED(type);

    ffStrbufSetS(name, "KDE Plasma");

    char versionBuf[256];
    ffParsePropFile("/usr/share/xsessions/plasma.desktop", "X-KDE-PluginInfo-Version=%[^\n]", versionBuf);

    ffStrbufSetS(version, versionBuf);
}

void ffPrintDesktopEnvironment(FFinstance* instance)
{
    FF_STRBUF_CREATE(sessionDesktop);
    ffStrbufAppendS(&sessionDesktop, getenv("XDG_CURRENT_DESKTOP"));

    if(sessionDesktop.length == 0)
        ffStrbufSetS(&sessionDesktop, getenv("XDG_SESSION_DESKTOP"));

    FF_STRBUF_CREATE(sessionVersion);
    FF_STRBUF_CREATE(sessionType);

    if(ffStrbufIgnCaseCompS(&sessionDesktop, "KDE") == 0)
        getKDE(&sessionDesktop, &sessionVersion, &sessionType);

    if(sessionType.length == 0)
    {
        const char* xdgSessionType = getenv("XDG_SESSION_TYPE");
        if(strcasecmp(xdgSessionType, "wayland") == 0)
            ffStrbufSetS(&sessionType, "Wayland");
        else if(strcasecmp(xdgSessionType, "x11") == 0)
            ffStrbufSetS(&sessionType, "X11");
        else if(strcasecmp(xdgSessionType, "tty") == 0)
            ffStrbufSetS(&sessionType, "TTY");
        else if(strcasecmp(xdgSessionType, "mir") == 0)
            ffStrbufSetS(&sessionType, "Mir");
        else if(xdgSessionType != NULL)
            ffStrbufSetS(&sessionType, xdgSessionType);
    }

    if(sessionDesktop.length == 0 && sessionType.length == 0)
    {
        ffPrintError(instance, FF_DE_MODULE_NAME, 0, &instance->config.deKey, &instance->config.deFormat, FF_DE_NUM_FORMAT_ARGS, "No relevant XDG_SESSION_* environment variable set");
        ffStrbufDestroy(&sessionDesktop);
        ffStrbufDestroy(&sessionVersion);
        ffStrbufDestroy(&sessionType);
        return;
    }

    if(instance->config.deFormat.length == 0)
    {

        ffPrintLogoAndKey(instance, FF_DE_MODULE_NAME, 0, &instance->config.deKey);

        if(sessionDesktop.length > 0)
        {
            ffStrbufWriteTo(&sessionDesktop, stdout);

            if(sessionVersion.length > 0)
            {
                putchar(' ');
                ffStrbufWriteTo(&sessionVersion, stdout);
            }

            if(sessionType.length > 0)
                putchar(' ');
        }

        if(sessionType.length > 0)
            printf("(%s)", sessionType.chars);

        putchar('\n');
    }
    else
    {
        ffPrintFormatString(instance, FF_DE_MODULE_NAME, 0, &instance->config.deKey, &instance->config.deFormat, NULL, FF_DE_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &sessionDesktop},
            {FF_FORMAT_ARG_TYPE_STRBUF, &sessionVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &sessionType}
        });
    }

    ffStrbufDestroy(&sessionDesktop);
    ffStrbufDestroy(&sessionVersion);
    ffStrbufDestroy(&sessionType);
}
