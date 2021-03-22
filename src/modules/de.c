#include "fastfetch.h"

static void getKDE(FFstrbuf* name, FFstrbuf* version, FFstrbuf* type)
{
    ffStrbufSetS(name, "KDE Plasma");

    char versionBuf[256];
    ffParsePropFile("/usr/share/xsessions/plasma.desktop", "X-KDE-PluginInfo-Version=%[^\n]", versionBuf);

    ffStrbufSetS(version, versionBuf);
}

void ffPrintDesktopEnvironment(FFinstance* instance)
{
    FFstrbuf sessionDesktop;
    ffStrbufInitS(&sessionDesktop, getenv("XDG_CURRENT_DESKTOP"));
    if(ffStrbufIsEmpty(&sessionDesktop))
        ffStrbufSetS(&sessionDesktop, getenv("XDG_SESSION_DESKTOP"));

    FF_STRBUF_CREATE(sessionVersion);
    FF_STRBUF_CREATE(sessionType);

    if(ffStrbufIgnCaseCompS(&sessionDesktop, "KDE") == 0)
        getKDE(&sessionDesktop, &sessionVersion, &sessionType);

    if(ffStrbufIsEmpty(&sessionType))
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

    if(ffStrbufIsEmpty(&instance->config.deFormat))
    {
        if(ffStrbufIsEmpty(&sessionDesktop) && ffStrbufIsEmpty(&sessionType))
        {
            ffPrintError(instance, "DE", "Neither DE nor Display Server could be determined");
            return;
        }

        ffPrintLogoAndKey(instance, "DE");

        if(!ffStrbufIsEmpty(&sessionDesktop))
        {
            ffStrbufWriteTo(&sessionDesktop, stdout);

            if(!ffStrbufIsEmpty(&sessionVersion))
            {
                putchar(' ');
                ffStrbufWriteTo(&sessionVersion, stdout);
            }

            if(!ffStrbufIsEmpty(&sessionType))
                putchar(' ');
        }

        if(!ffStrbufIsEmpty(&sessionType))
            printf("(%s)", sessionType.chars);

        putchar('\n');
    }
    else
    {
        ffPrintLogoAndKey(instance, "DE");

        FF_STRBUF_CREATE(de);

        ffParseFormatString(&de, &instance->config.deFormat, 3,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &sessionDesktop},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &sessionVersion},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, &sessionType}
        );

        ffStrbufPutTo(&de, stdout);
        ffStrbufDestroy(&de);
    }
    ffStrbufDestroy(&sessionType);
    ffStrbufDestroy(&sessionVersion);
    ffStrbufDestroy(&sessionDesktop);
}