#include "fastfetch.h"

static void printKDE()
{
    char version[256];
    ffParsePropFile("/usr/share/xsessions/plasma.desktop", "X-KDE-PluginInfo-Version=%[^\n]", version);
    if(version[0] == '\0')
    {
        printf("KDE Plasma ");
        return;
    }

    printf("KDE Plasma %s", version);
}

void ffPrintDesktopEnvironment(FFinstance* instance)
{
    #ifdef FASTFETCH_BUILD_FLASHFETCH
    if(ffPrintCustomValue(instance, "DE"))
        return;
    #endif // FASTFETCH_BUILD_FLASHFETCH

    const char* currentDesktop = getenv("XDG_CURRENT_DESKTOP");
    if(currentDesktop == NULL)
    {
        ffPrintError(instance, "DE", "getenv(\"XDG_CURRENT_DESKTOP\") == NULL");
        return;
    }

    ffPrintLogoAndKey(instance, "DE");

    if(strcasecmp(currentDesktop, "KDE") == 0)
        printKDE();
    else
        printf("%s", currentDesktop);

    const char* sessionType = getenv("XDG_SESSION_TYPE");
    if(sessionType == NULL)
        putchar('\n');
    else if(strcasecmp(sessionType, "wayland") == 0)
        puts(" (Wayland)");
    else if(strcasecmp(sessionType, "x11") == 0)
        puts(" (X11)");
    else if(strcasecmp(sessionType, "tty") == 0)
        puts(" TTY");
    else if(strcasecmp(sessionType, "mir") == 0)
        puts(" (Mir)");
    else
        printf(" (%s)\n", sessionType);
}